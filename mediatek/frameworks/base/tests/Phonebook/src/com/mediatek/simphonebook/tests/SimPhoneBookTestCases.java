/*
 * Copyright (C) 2006 The Android Open Source Project
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

package com.mediatek.simphonebook.tests;

import android.content.Context;
import android.content.Intent;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.test.AndroidTestCase;
import android.text.TextUtils;

import android.util.Log;
import java.util.List;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.IIccPhoneBook;
import com.android.internal.telephony.uicc.AdnRecord;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;
import com.android.internal.telephony.uicc.IccCardStatus.CardState;

import com.mediatek.common.telephony.AlphaTag;
import com.mediatek.common.telephony.UsimGroup;


public class SimPhoneBookTestCases extends AndroidTestCase {

    private static final String TAG = "SimPhoneBookTest";
    private List<AdnRecord> adnRecordList = null;
    private boolean isPhbReady = false;

    private boolean isSimInserted(int simId) throws Exception  {
        UiccCard uiccCard = UiccController.getInstance(simId).getUiccCard();

        if (uiccCard != null && (uiccCard.getCardState() == CardState.CARDSTATE_PRESENT)) {
            logd("isSimInserted(" + simId + "):" + uiccCard.getCardState());
            return true;
        }

        return false;
        
        //ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        //if (iTel != null) {
        //    logd("isSimInserted(" + simId + "):" + iTel.isSimInsert(simId));
        //    return iTel.isSimInsert(simId);
        //}

        //return false;
    }
    
    private boolean isUsim() throws Exception {
        UiccCardApplication uiccApp = UiccController.getInstance(0).getUiccCardApplication(UiccController.APP_FAM_3GPP);

        if (uiccApp != null && (uiccApp.getType() == AppType.APPTYPE_USIM)) {
            logd("isUsim:" + uiccApp.getType());
            return true;
        }

        logd("SIM1 is not Usim or there is no SIM card inserted.");    

        return false;
        
        //ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        //if (iTel != null) {
        //    // TODO: always return sim1 card type
        //    String str = iTel.getIccCardType();
        //    logd("isUsim:" + str);
        //    return "USIM".equals(str);
        //}
        //return false;
    }

    private boolean isPhbReady() throws Exception {
        IIccPhoneBook simPhoneBook =
                    IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
        assertNotNull(simPhoneBook);

        int sleeptime = 500;
        while (sleeptime <= 2000 && !isPhbReady) {
            logd("Phonebook is not ready, sleep time = " + sleeptime);
            Thread.sleep(sleeptime);
            isPhbReady = simPhoneBook.isPhbReady();
            sleeptime += 500;
        }

        if (!isPhbReady) {
            logd("Phonebook still not ready after sleeping over 2 second..., isSimInserted(0):" + isSimInserted(0));
        }

        logd("isPhbReady = " + isPhbReady);

        return isPhbReady;
    }

    
    private void logd(String str) {
        Log.d(TAG, "" + str);
    }
    
    private void removeAllAdnCache() {
        Context ctx = getContext();
        Intent intent = new Intent("com.mediatek.dm.LAWMO_WIPE");
        ctx.sendBroadcast(intent);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException ex) {
            logd(ex.getMessage());
        }
    }
    
    public void testBasic() throws Exception {
        logd("testBasic begin");
        removeAllAdnCache();

        if (!isPhbReady()) {
            return;
        }
        
        IIccPhoneBook simPhoneBook =
                IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
        assertNotNull(simPhoneBook);

        int size[] = simPhoneBook.getAdnRecordsSize(IccConstants.EF_ADN);
        assertNotNull(size);
        assertEquals(3, size.length);
        assertEquals(size[0] * size[2], size[1]);
        //assertTrue(size[2] >= 100);

        adnRecordList = simPhoneBook.getAdnRecordsInEf(IccConstants.EF_ADN);
        // do it twice cause the second time shall read from cache only
        adnRecordList = simPhoneBook.getAdnRecordsInEf(IccConstants.EF_ADN);

        int sleeptime = 500;        
        while (adnRecordList == null && sleeptime <= 5000) {            
            logd("adnRecordList is null, sleep time = " + sleeptime);            
            Thread.sleep(sleeptime);            
            adnRecordList = simPhoneBook.getAdnRecordsInEf(IccConstants.EF_ADN);            
            sleeptime += 500;        
        }            

        if (adnRecordList == null) {            
            logd("adnRecordList still null after sleeping over 2 second, return directly");            
        }   

        logd("testBasic end");
    }
    
    public void testUpdateContacts() throws Exception{
        logd("testUpdateContacts");	

        if (!isPhbReady()) {
            return;
        } else if (adnRecordList == null) {
            IIccPhoneBook simPhoneBook =
                IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
            adnRecordList = ((simPhoneBook == null) ? null : simPhoneBook.getAdnRecordsInEf(IccConstants.EF_ADN));
        }
        
        if (adnRecordList == null) {
            logd("adnRecordList still null after phb ready...");    
            return;
        }

        logd("adnRecordList is not null, start to do test...");  

        IIccPhoneBook simPhoneBook =
                IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
        assertNotNull(simPhoneBook);
  
        // Test for phone book update
        int adnIndex, listIndex = 0;
        AdnRecord originalAdn = null;
        // We need to maintain the state of the SIM before and after the test.
        // Since this test doesn't mock the SIM we try to get a valid ADN record,
        // for 3 tries and if this fails, we bail out. 
        for (adnIndex = 3 ; adnIndex >= 1; adnIndex--) {
            listIndex = adnIndex - 1; // listIndex is zero based.
            originalAdn = adnRecordList.get(listIndex);
            assertNotNull("Original Adn is Null.", originalAdn);
            assertNotNull("Original Adn alpha tag is null.", originalAdn.getAlphaTag());
            assertNotNull("Original Adn number is null.", originalAdn.getNumber());
            
            if (originalAdn.getNumber().length() > 0 &&  
                originalAdn.getAlphaTag().length() > 0) {   
                break;
            }
        }
        if (adnIndex == 0) return;
        
        AdnRecord emptyAdn = new AdnRecord("", "");
        AdnRecord firstAdn = new AdnRecord("John", "4085550101");
        AdnRecord secondAdn = new AdnRecord("Andy", "6505550102");
        String pin2 = null;

        // udpate by index
        logd("testUpdateContacts udpate by index");
        boolean success = simPhoneBook.updateAdnRecordsInEfByIndex(IccConstants.EF_ADN,
                firstAdn.getAlphaTag(), firstAdn.getNumber(), adnIndex, pin2);
        adnRecordList = simPhoneBook.getAdnRecordsInEf(IccConstants.EF_ADN);
         AdnRecord tmpAdn = adnRecordList.get(listIndex);
        assertTrue(success);
        assertTrue(firstAdn.isEqual(tmpAdn));

        // replace by search
        logd("testUpdateContacts replace by search");
        int result = simPhoneBook.updateAdnRecordsInEfBySearchWithError(IccConstants.EF_ADN,
                firstAdn.getAlphaTag(), firstAdn.getNumber(),
                secondAdn.getAlphaTag(), secondAdn.getNumber(), pin2);
        adnRecordList = simPhoneBook.getAdnRecordsInEf(IccConstants.EF_ADN);
        tmpAdn = adnRecordList.get(listIndex);
        assertTrue(result > 0);
        assertFalse(firstAdn.isEqual(tmpAdn));
        assertTrue(secondAdn.isEqual(tmpAdn));

        // erase be search
        logd("testUpdateContacts erase be search");
        result = simPhoneBook.updateAdnRecordsInEfBySearchWithError(IccConstants.EF_ADN,
                secondAdn.getAlphaTag(), secondAdn.getNumber(),
                emptyAdn.getAlphaTag(), emptyAdn.getNumber(), pin2);
        adnRecordList = simPhoneBook.getAdnRecordsInEf(IccConstants.EF_ADN);
        tmpAdn = adnRecordList.get(listIndex);
        assertTrue(result > 0);
        assertTrue(tmpAdn.isEmpty());

        // restore the orginial adn
        logd("testUpdateContacts restore the orginial adn");
        success = simPhoneBook.updateAdnRecordsInEfByIndex(IccConstants.EF_ADN,
                originalAdn.getAlphaTag(), originalAdn.getNumber(), adnIndex,
                pin2);
        adnRecordList = simPhoneBook.getAdnRecordsInEf(IccConstants.EF_ADN);
        tmpAdn = adnRecordList.get(listIndex);
        assertTrue(success);
        assertTrue(originalAdn.isEqual(tmpAdn));
    }
    
    public void testGetPhonebookMemStorageExt() throws Exception{
        logd("testGetPhonebookMemStorageExt");
        if (!isUsim() || !isPhbReady()) {
            return;
        }
        IIccPhoneBook simPhoneBook =
            IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
        assertNotNull(simPhoneBook);
        assertNotNull(simPhoneBook.getPhonebookMemStorageExt());
    }
    
    public void testGetSneRecordLen() throws Exception{
        logd("testGetSneRecordLen");
        if (!isUsim() || !isPhbReady()) {
            return;
        }
        IIccPhoneBook simPhoneBook =
            IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
        assertNotNull(simPhoneBook);
        int len = simPhoneBook.getSneRecordLen();
        assertTrue(len >= -1);
    }
    
    public void testGroup() throws Exception {
        logd("testGroup");
        if (!isUsim() || !isPhbReady()) {
            return;
        }
        IIccPhoneBook simPhoneBook =
            IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
        assertNotNull(simPhoneBook);
        int count = simPhoneBook.getUsimGrpMaxCount();
        int nameLen = simPhoneBook.getUsimGrpMaxNameLen();
        logd("max group count:" + count + ",max name len:" + nameLen);
        if (count <=0 || nameLen <= 0) return;
        
        List<UsimGroup> list = simPhoneBook.getUsimGroups();
        if (list == null) return;
        int size = list.size();
        UsimGroup origin = null;
        for (int i = 0; i < size;i++) {
            origin = list.get(i);
            String groupName = origin.getAlphaTag();
            int index  = origin.getRecordIndex();
            if (!TextUtils.isEmpty(groupName) && index > 0) {
                break;
            }
        }

        int newIndex = simPhoneBook.insertUsimGroup("MM");
        logd("insert usim group index " + newIndex);
        if (newIndex > 0) {
            if (origin.getRecordIndex() <= 0) {
                origin = new UsimGroup(newIndex, "MM");
            }
        }

        logd("testGroup update group");
        String originName = origin.getAlphaTag();
        int originIndex  = origin.getRecordIndex();
        assertTrue(simPhoneBook.updateUsimGroup(originIndex, "HM") == originIndex); 
        String tem = simPhoneBook.getUsimGroupById(originIndex);
        assertTrue("HM".equals(tem));
        
        assertTrue(simPhoneBook.hasExistGroup("HM") == originIndex);
        logd("testGroup restore group " + originName);
        simPhoneBook.updateUsimGroup(originIndex, originName);
        if (newIndex > 0) {
            assertTrue(simPhoneBook.removeUsimGroupById(newIndex));
        }
    }
    
    public void testHasSne() throws Exception{
        logd("testHasSne");
        if (!isUsim() || !isPhbReady()) {
            return;
        }
        IIccPhoneBook simPhoneBook =
            IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
        assertNotNull(simPhoneBook);
        assertNotNull(Boolean.valueOf(simPhoneBook.hasSne()));
        logd("sne len:" + simPhoneBook.getSneRecordLen());
    }
    

    public void testIsPhbReady() throws Exception {
        assertTrue(isPhbReady());
    }
    
    public void testUsimAas() throws Exception{
        logd("testUsimAas");
        if (!isUsim() || !isPhbReady()) {
            return;
        }
        IIccPhoneBook simPhoneBook =
            IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
        assertNotNull(simPhoneBook);
        
        if (simPhoneBook.getAnrCount() <= 0) {
            return;
        }

        logd("getUsimAasList");

        List<AlphaTag> list = simPhoneBook.getUsimAasList();
        if (list == null) return;
        int size = list.size();
        AlphaTag origin = null;
        for (int index = 0; index < size; index++) {
            origin = list.get(index);
            String name = origin.getAlphaTag();
            int ind = origin.getRecordIndex();
            if (!TextUtils.isEmpty(name) && ind > 0) {
                break;
            }
        }
        if (origin == null) {
            logd("null origin aas");
            return;
        }
        String testAas = "TestAas";
        int newIndex = simPhoneBook.insertUsimAas(testAas);
        if (newIndex > 0) {
            if (origin.getRecordIndex() <= 0) {
                origin = new AlphaTag(newIndex, testAas, 0);
            }
        }
        
        String originName = origin.getAlphaTag();
        int originIndex = origin.getRecordIndex();
        int originPbr = origin.getPbrIndex();
        logd("testUsimAas update");
        assertTrue(simPhoneBook.updateUsimAas(originPbr, originIndex, "first_test"));
        String tem = simPhoneBook.getUsimAasById(originIndex);
        assertTrue("first_test".equals(tem));
        logd("testUsimAas restore");
        assertTrue(simPhoneBook.updateUsimAas(originPbr, originIndex, originName));
        
        if (newIndex > 0) {
            logd("testUsimAas remove");
            assertTrue(simPhoneBook.removeUsimAasById(newIndex, 0));
        }
        
        int count = simPhoneBook.getUsimAasMaxCount();
        int nameLen = simPhoneBook.getUsimAasMaxNameLen();
        logd("aas count:" + count + ", name len:" + nameLen);
    }
    
    public void testUsimBasic() throws Exception {
        logd("testUsimBasic");
        if (!isUsim() || !isPhbReady()) {
            return;
        } else if (adnRecordList == null) {
            IIccPhoneBook simPhoneBook =
                IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
            adnRecordList = ((simPhoneBook == null) ? null : simPhoneBook.getAdnRecordsInEf(IccConstants.EF_ADN));
        }
       
        if (adnRecordList == null) {
            logd("adnRecordList still null after phb ready...");    
            return;
        }

        logd("adnRecordList is not null, start to do test...");  
        
        IIccPhoneBook simPhoneBook =
            IIccPhoneBook.Stub.asInterface(ServiceManager.getService("simphonebook"));
        assertNotNull(simPhoneBook);
        
        List<AdnRecord> adnRecordList = simPhoneBook.getAdnRecordsInEf(IccConstants.EF_ADN);
        // do it twice cause the second time shall read from cache only
        assertNotNull(adnRecordList);

        // Test for phone book update
        int adnIndex, listIndex = 0;
        AdnRecord originalAdn = null;
        // We need to maintain the state of the SIM before and after the test.
        // Since this test doesn't mock the SIM we try to get a valid ADN record,
        // for 3 tries and if this fails, we bail out. 
        for (adnIndex = 3 ; adnIndex >= 1; adnIndex--) {
            listIndex = adnIndex - 1; // listIndex is zero based.
            originalAdn = adnRecordList.get(listIndex);
            assertNotNull("Original Adn is Null.", originalAdn);
            assertNotNull("Original Adn alpha tag is null.", originalAdn.getAlphaTag());
            assertNotNull("Original Adn number is null.", originalAdn.getNumber());
            
            if (originalAdn.getNumber().length() > 0 &&  
                originalAdn.getAlphaTag().length() > 0) {   
                break;
            }
        }
        if (adnIndex == 0) return;
        
        AdnRecord emptyAdn = new AdnRecord("", "");
        AdnRecord firstAdn = new AdnRecord("John", "4085550101");
        firstAdn.setAnr("443232211");
        firstAdn.setEmails(new String[]{"Hohn@test.com"});
        AdnRecord secondAdn = new AdnRecord("Andy", "6505550102");
        secondAdn.setAnr("99887766");
        secondAdn.setEmails(new String[]{"Andy@test.com"});
        String pin2 = null;
        
        logd("testUsimBasic replace by index");
        int result = simPhoneBook.updateUsimPBRecordsByIndexWithError(IccConstants.EF_ADN, firstAdn, adnIndex);
        assertTrue(result > 0);
        
        AdnRecord tem = adnRecordList.get(listIndex);
        assertTrue(firstAdn.equals(tem));
        logd("testUsimBasic replace by search");
        result = simPhoneBook.updateUsimPBRecordsBySearchWithError(IccConstants.EF_ADN, firstAdn, secondAdn);
        
        assertTrue(result > 0);
        
        logd("testUsimBasic erase by search");
        result = simPhoneBook.updateUsimPBRecordsInEfBySearchWithError(IccConstants.EF_ADN,
                secondAdn.getAlphaTag(), secondAdn.getNumber(), secondAdn.getAdditionalNumber(), null, null,
                emptyAdn.getAlphaTag(), emptyAdn.getNumber(),"", null, null);
        assertTrue(result > 0);
        
        assertTrue(adnRecordList.get(listIndex).isEmpty());
        
        logd("testUsimBasic restore the orginial adn");
        
        result = simPhoneBook.updateUsimPBRecordsInEfByIndexWithError(IccConstants.EF_ADN,originalAdn.getAlphaTag(), originalAdn.getNumber(),
                originalAdn.getAdditionalNumber(), originalAdn.getGrpIds(), originalAdn.getEmails(), adnIndex);
        assertTrue(result > 0);
    }
}
