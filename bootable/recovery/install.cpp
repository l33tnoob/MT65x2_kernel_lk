/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"
#include "install.h"
#include "mincrypt/rsa.h"
#include "minui/minui.h"
#include "minzip/SysUtil.h"
#include "minzip/Zip.h"
#include "mtdutils/mounts.h"
#include "mtdutils/mtdutils.h"
#include "roots.h"
#include "verifier.h"
#include "ui.h"
#if 1 //tonykuo 2013-11-12
#include "mincrypt/rsa.h"
#include "mincrypt/sha.h"
#include "mincrypt/sha256.h"
#endif
#if 1 //wschen 2012-07-10
#include "bootloader.h"
#endif

#ifdef SUPPORT_DATA_BACKUP_RESTORE //wschen 2011-03-09 
#include "backup_restore.h"
#endif

#ifdef SUPPORT_SBOOT_UPDATE
#include "sec/sec.h"
#endif

extern RecoveryUI* ui;
int update_from_data;


#if 1 //wschen 2012-07-10
static void reset_mark_block(void)
{
    // Reset the bootloader message to revert to a normal main system boot.
    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));
    set_bootloader_message(&boot);
    sync();
}
#endif

#define ASSUMED_UPDATE_BINARY_NAME  "META-INF/com/google/android/update-binary"
#define PUBLIC_KEYS_FILE "/res/keys"

#ifdef EXTERNAL_MODEM_UPDATE
#define BROMLITE_NAME  "META-INF/com/google/android/bromLite-binary"
#define BROMLITE_PATH  "/tmp/bromLite-binary"
#define MODEM_FILE_PATH  "/system/etc/firmware/modem"
#endif

// Default allocation of progress bar segments to operations
static const int VERIFICATION_PROGRESS_TIME = 60;
static const float VERIFICATION_PROGRESS_FRACTION = 0.25;
static const float DEFAULT_FILES_PROGRESS_FRACTION = 0.4;
static const float DEFAULT_IMAGE_PROGRESS_FRACTION = 0.1;

// If the package contains an update binary, extract it and run it.
static int
try_update_binary(const char *path, ZipArchive *zip, int* wipe_cache) {
    const ZipEntry* binary_entry =
            mzFindZipEntry(zip, ASSUMED_UPDATE_BINARY_NAME);
    if (binary_entry == NULL) {
        mzCloseZipArchive(zip);
        return INSTALL_CORRUPT;
    }

    const char* binary = "/tmp/update_binary";
    unlink(binary);
    int fd = creat(binary, 0755);
    if (fd < 0) {
        mzCloseZipArchive(zip);
        LOGE("Can't make %s\n", binary);
        return INSTALL_ERROR;
    }
    bool ok = mzExtractZipEntryToFile(zip, binary_entry, fd);
    close(fd);
    mzCloseZipArchive(zip);

    if (!ok) {
        LOGE("Can't copy %s\n", ASSUMED_UPDATE_BINARY_NAME);
        return INSTALL_ERROR;
    }

    int pipefd[2];
    pipe(pipefd);

    // When executing the update binary contained in the package, the
    // arguments passed are:
    //
    //   - the version number for this interface
    //
    //   - an fd to which the program can write in order to update the
    //     progress bar.  The program can write single-line commands:
    //
    //        progress <frac> <secs>
    //            fill up the next <frac> part of of the progress bar
    //            over <secs> seconds.  If <secs> is zero, use
    //            set_progress commands to manually control the
    //            progress of this segment of the bar
    //
    //        set_progress <frac>
    //            <frac> should be between 0.0 and 1.0; sets the
    //            progress bar within the segment defined by the most
    //            recent progress command.
    //
    //        firmware <"hboot"|"radio"> <filename>
    //            arrange to install the contents of <filename> in the
    //            given partition on reboot.
    //
    //            (API v2: <filename> may start with "PACKAGE:" to
    //            indicate taking a file from the OTA package.)
    //
    //            (API v3: this command no longer exists.)
    //
    //        ui_print <string>
    //            display <string> on the screen.
    //
    //   - the name of the package zip file.
    //

    const char** args = (const char**)malloc(sizeof(char*) * 5);
    args[0] = binary;
    args[1] = EXPAND(RECOVERY_API_VERSION);   // defined in Android.mk
    char* temp = (char*)malloc(10);
    sprintf(temp, "%d", pipefd[1]);
    args[2] = temp;
    args[3] = (char*)path;
    args[4] = NULL;

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        execv(binary, (char* const*)args);
        fprintf(stdout, "E:Can't run %s (%s)\n", binary, strerror(errno));
        _exit(-1);
    }
    close(pipefd[1]);

    *wipe_cache = 0;

    char buffer[1024];
    FILE* from_child = fdopen(pipefd[0], "r");
    while (fgets(buffer, sizeof(buffer), from_child) != NULL) {
        char* command = strtok(buffer, " \n");
        if (command == NULL) {
            continue;
        } else if (strcmp(command, "progress") == 0) {
            char* fraction_s = strtok(NULL, " \n");
            char* seconds_s = strtok(NULL, " \n");

            float fraction = strtof(fraction_s, NULL);
            int seconds = strtol(seconds_s, NULL, 10);

            ui->ShowProgress(fraction * (1-VERIFICATION_PROGRESS_FRACTION), seconds);
        } else if (strcmp(command, "set_progress") == 0) {
            char* fraction_s = strtok(NULL, " \n");
            float fraction = strtof(fraction_s, NULL);
            ui->SetProgress(fraction);
        } else if (strcmp(command, "ui_print") == 0) {
            char* str = strtok(NULL, "\n");
            if (str) {
                ui->Print("%s", str);
            } else {
                ui->Print("\n");
            }
            fflush(stdout);
        } else if (strcmp(command, "wipe_cache") == 0) {
            *wipe_cache = 1;
#if 1 //wschen 2012-07-25
        } else if (strcmp(command, "special_factory_reset") == 0) {
            *wipe_cache = 2;
#endif
        } else if (strcmp(command, "clear_display") == 0) {
            ui->SetBackground(RecoveryUI::NONE);
        } else {
            LOGE("unknown command [%s]\n", command);
        }
    }
    fclose(from_child);

    int status;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        LOGE("Error in %s\n(Status %d)\n", path, WEXITSTATUS(status));
        return INSTALL_ERROR;
    }

#ifdef SUPPORT_DATA_BACKUP_RESTORE //wschen 2011-03-09 
//Skip userdata restore if updating from /data with no /data layout change
if(!usrdata_changed && update_from_data){
    ui->Print("/data offset remains the same no need to restore usrdata\n");
}else{
    if (part_size_changed) {
        if (ensure_path_mounted("/sdcard") != 0) {
            LOGE("Can't mount %s\n", path);
            return INSTALL_NO_SDCARD;
        }

        if (userdata_restore(backup_path, 1)) {
            return INSTALL_FILE_SYSTEM_ERROR;
        }
    }
}
#endif //SUPPORT_DATA_BACKUP_RESTORE

    /* ----------------------------- */
    /* SECURE BOOT UPDATE            */    
    /* ----------------------------- */            
#ifdef SUPPORT_SBOOT_UPDATE
    sec_update(false);
#endif

    return INSTALL_SUCCESS;
}

#ifdef EXTERNAL_MODEM_UPDATE
#define EXT_MD_IOC_MAGIC		'E'
#define EXT_MD_IOCTL_R8_DOWNLOAD   	_IO(EXT_MD_IOC_MAGIC, 106)
#define EXT_MD_MONITOR_DEV "/dev/ext_md_ctl0"

static int
try_update_modem(const char *path) {
    int i, fd, pipe_fd[2], status;
    pid_t pid;
    ZipEntry *temp_entry = NULL;
    FILE *from_child;
    ZipArchive zip;

    //unzip bromlite to /tmp
    status = mzOpenZipArchive(path, &zip);
    if (status != 0) {
        LOGE("Can't open %s\n(%s)\n", path, status != -1 ? strerror(status) : "bad");
        return INSTALL_CORRUPT;
    }    
    temp_entry = (ZipEntry *)mzFindZipEntry(&zip, BROMLITE_NAME);
    if (temp_entry == NULL) {
        LOGE("Can't find %s, maybe don't need to upgrade modem \n", BROMLITE_NAME);
        mzCloseZipArchive(&zip);     
        return INSTALL_SUCCESS;
    }
    unlink(BROMLITE_PATH);
    fd = creat(BROMLITE_PATH, 0755);
    if (fd < 0) {
        mzCloseZipArchive(&zip);
        LOGE("Can't make %s\n", BROMLITE_PATH);
        return INSTALL_ERROR;
    }
    bool ok = mzExtractZipEntryToFile(&zip, temp_entry, fd);
    close(fd);
    mzCloseZipArchive(&zip);
    if (!ok) {
        LOGE("Can't copy %s\n", BROMLITE_NAME);
        return INSTALL_ERROR;
    }

    //update modem from MODEM_FILE_PATH
    ensure_path_mounted("/system");
    pid  = fork();
    if(pid == 0){
        execl(BROMLITE_PATH, "bromLite-binary", MODEM_FILE_PATH, 0); 
        fprintf(stdout, "E:Can't run %s (%s)\n", BROMLITE_PATH, strerror(errno));
        _exit(-1);
    }
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        LOGE("Error in %s\n(Status %d)\n", path, WEXITSTATUS(status));
        return INSTALL_ERROR;
    }

    return INSTALL_SUCCESS;
}
#endif 

#if 0
// Reads a file containing one or more public keys as produced by
// DumpPublicKey:  this is an RSAPublicKey struct as it would appear
// as a C source literal, eg:
//
//  "{64,0xc926ad21,{1795090719,...,-695002876},{-857949815,...,1175080310}}"
//
// For key versions newer than the original 2048-bit e=3 keys
// supported by Android, the string is preceded by a version
// identifier, eg:
//
//  "v2 {64,0xc926ad21,{1795090719,...,-695002876},{-857949815,...,1175080310}}"
//
// (Note that the braces and commas in this example are actual
// characters the parser expects to find in the file; the ellipses
// indicate more numbers omitted from this example.)
//
// The file may contain multiple keys in this format, separated by
// commas.  The last key must not be followed by a comma.
//
// A Certificate is a pair of an RSAPublicKey and a particular hash
// (we support SHA-1 and SHA-256; we store the hash length to signify
// which is being used).  The hash used is implied by the version number.
//
//       1: 2048-bit RSA key with e=3 and SHA-1 hash
//       2: 2048-bit RSA key with e=65537 and SHA-1 hash
//       3: 2048-bit RSA key with e=3 and SHA-256 hash
//       4: 2048-bit RSA key with e=65537 and SHA-256 hash
//
// Returns NULL if the file failed to parse, or if it contain zero keys.
Certificate*
load_keys(const char* filename, int* numKeys) {
    Certificate* out = NULL;
    *numKeys = 0;

    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        LOGE("opening %s: %s\n", filename, strerror(errno));
        goto exit;
    }

    {
        int i;
        bool done = false;
        while (!done) {
            ++*numKeys;
            out = (Certificate*)realloc(out, *numKeys * sizeof(Certificate));
            Certificate* cert = out + (*numKeys - 1);
            cert->public_key = (RSAPublicKey*)malloc(sizeof(RSAPublicKey));

            char start_char;
            if (fscanf(f, " %c", &start_char) != 1) goto exit;
            if (start_char == '{') {
                // a version 1 key has no version specifier.
                cert->public_key->exponent = 3;
                cert->hash_len = SHA_DIGEST_SIZE;
            } else if (start_char == 'v') {
                int version;
                if (fscanf(f, "%d {", &version) != 1) goto exit;
                switch (version) {
                    case 2:
                        cert->public_key->exponent = 65537;
                        cert->hash_len = SHA_DIGEST_SIZE;
                        break;
                    case 3:
                        cert->public_key->exponent = 3;
                        cert->hash_len = SHA256_DIGEST_SIZE;
                        break;
                    case 4:
                        cert->public_key->exponent = 65537;
                        cert->hash_len = SHA256_DIGEST_SIZE;
                        break;
                    default:
                        goto exit;
                }
            }

            RSAPublicKey* key = cert->public_key;
            if (fscanf(f, " %i , 0x%x , { %u",
                       &(key->len), &(key->n0inv), &(key->n[0])) != 3) {
                goto exit;
            }
            if (key->len != RSANUMWORDS) {
                LOGE("key length (%d) does not match expected size\n", key->len);
                goto exit;
            }
            for (i = 1; i < key->len; ++i) {
                if (fscanf(f, " , %u", &(key->n[i])) != 1) goto exit;
            }
            if (fscanf(f, " } , { %u", &(key->rr[0])) != 1) goto exit;
            for (i = 1; i < key->len; ++i) {
                if (fscanf(f, " , %u", &(key->rr[i])) != 1) goto exit;
            }
            fscanf(f, " } } ");

            // if the line ends in a comma, this file has more keys.
            switch (fgetc(f)) {
            case ',':
                // more keys to come.
                break;

            case EOF:
                done = true;
                break;

            default:
                LOGE("unexpected character between keys\n");
                goto exit;
            }

            LOGI("read key e=%d hash=%d\n", key->exponent, cert->hash_len);
        }
    }

    fclose(f);
    return out;

exit:
    if (f) fclose(f);
    free(out);
    *numKeys = 0;
    return NULL;
}
#endif 
static int
really_install_package(const char *path, int* wipe_cache)
{
    ui->SetBackground(RecoveryUI::INSTALLING_UPDATE);
#if 0 //wschen 2012-07-10
    ui->Print("Finding update package...\n");
#else
    LOGI("Finding update package...\n");
#endif
    ui->SetProgressType(RecoveryUI::INDETERMINATE);
    LOGI("Update location: %s\n", path);

    if (ensure_path_mounted(path) != 0) {
        LOGE("Can't mount %s\n", path);
#if 0 //wschen 2012-07-10
        return INSTALL_CORRUPT;
#else
        reset_mark_block();
        return INSTALL_NO_SDCARD;
#endif
    }

#if 0 //wschen 2012-07-10
    ui->Print("Opening update package...\n");
#else
    LOGI("Opening update package...\n");
#endif

    int numKeys;
     Certificate* loadedKeys = load_keys(PUBLIC_KEYS_FILE, &numKeys);
    if (loadedKeys == NULL) {
        LOGE("Failed to load keys\n");
#if 0 //wschen 2012-07-10
        return INSTALL_CORRUPT;
#else
        reset_mark_block();
        return INSTALL_NO_KEY;
#endif
    }
    LOGI("%d key(s) loaded from %s\n", numKeys, PUBLIC_KEYS_FILE);

    // Give verification half the progress bar...
#if 0 //wschen 2012-07-10
    ui->Print("Verifying update package...\n");
#else
    LOGI("Verifying update package...\n");
#endif
    ui->SetProgressType(RecoveryUI::DETERMINATE);
    ui->ShowProgress(VERIFICATION_PROGRESS_FRACTION, VERIFICATION_PROGRESS_TIME);

    int err;
    err = verify_file(path, loadedKeys, numKeys);
    free(loadedKeys);
    LOGI("verify_file returned %d\n", err);
    if (err != VERIFY_SUCCESS) {
        LOGE("signature verification failed\n");
#if 0 //wschen 2012-07-10
        return INSTALL_CORRUPT;
#else
        reset_mark_block();
        return INSTALL_SIGNATURE_ERROR;
#endif
    }

    /* Try to open the package.
     */
    ZipArchive zip;
    err = mzOpenZipArchive(path, &zip);
    if (err != 0) {
        LOGE("Can't open %s\n(%s)\n", path, err != -1 ? strerror(err) : "bad");
#if 1 //wschen 2012-07-10
        reset_mark_block();
#endif
        return INSTALL_CORRUPT;
    }

#ifdef SUPPORT_DATA_BACKUP_RESTORE //wschen 2011-03-09 
    update_from_data = 0;

    Volume* v = volume_for_path(path);
    if (strcmp(v->mount_point, "/data") == 0) {
	update_from_data = 1;
    }

    if (check_part_size(&zip, update_from_data) != 0) {
        reset_mark_block();
        return INSTALL_ERROR;
    }
#endif //SUPPORT_DATA_BACKUP_RESTORE
	
    /* ----------------------------- */
    /* SECURE BOOT CHECK             */    
    /* ----------------------------- */
#ifdef SUPPORT_SBOOT_UPDATE    
    if(0 != (err=sec_verify_img_info(&zip,false)))
    {
        return INSTALL_SECURE_CHECK_FAIL;
    }
    sec_mark_status(false);
#endif

    /* Verify and install the contents of the package.
     */
    ui->Print("Installing update...\n");
    err = try_update_binary(path, &zip, wipe_cache);
    if(err != INSTALL_SUCCESS)
        return err;

#ifdef EXTERNAL_MODEM_UPDATE
    ui->Print("Installing update Modem...\n");
    err = try_update_modem(path);
    if(err != INSTALL_SUCCESS)
    {
        LOGE("try_update_modem fail \n");
        return err;
    }
#endif
    return INSTALL_SUCCESS;
}

int
install_package(const char* path, int* wipe_cache, const char* install_file)
{
    FILE* install_log = fopen_path(install_file, "w");
    if (install_log) {
        fputs(path, install_log);
        fputc('\n', install_log);
    } else {
        LOGE("failed to open last_install: %s\n", strerror(errno));
    }
    int result = really_install_package(path, wipe_cache);
    if (install_log) {
        fputc(result == INSTALL_SUCCESS ? '1' : '0', install_log);
        fputc('\n', install_log);
        fclose(install_log);
    }
    return result;
}
