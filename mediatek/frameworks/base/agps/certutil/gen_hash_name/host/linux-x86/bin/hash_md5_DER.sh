#!/bin/sh

if IsUtilBuilt=`./../../../../../../../../out/host/linux-x86/bin/certutil version`
then
  # Use the new build version
  my_certutil=./../../../../../../../../out/host/linux-x86/bin/certutil
else
  # Use the prebuilt version
  my_certutil=./bin/certutil
fi

mkdir 0_der_md5
COUNTER=0
for cert in `ls CERT`
  do
   echo =============================
   echo CERT/${cert}
   echo =============================
   echo --- Method 1 ---
   echo ${my_certutil} x509 -subject_hash_old -noout -in CERT/${cert}
  if hash_name=`${my_certutil} x509 -subject_hash_old -noout -in CERT/${cert}`
  then
     echo hash_name=$hash_name
     ${my_certutil} x509 -in CERT/${cert} -out 0_der_md5/${hash_name}.0 -outform DER
  else
     echo --- Method 2 instead ---
     echo ${my_certutil} x509 -subject_hash_old -noout -in CERT/${cert} -inform DER
     if hash_name=`${my_certutil} x509 -subject_hash_old -noout -in CERT/${cert} -inform DER`
     then
       echo hash_name=$hash_name
       ${my_certutil} x509 -in CERT/${cert} -inform DER -out 0_der_md5/${hash_name}.0 -outform DER
     else
       echo !!! Method 3 instead !!!
       echo ${my_certutil} x509 -subject_hash_old -noout -in CERT/${cert} -inform PEM
       if hash_name=`${my_certutil} x509 -subject_hash_old -noout -in CERT/${cert} -inform PEM`
       then
         echo hash_name=$hash_name
         ${my_certutil} x509 -in CERT/${cert} -inform PEM -out 0_der_md5/${hash_name}.0 -outform DER
       fi
     fi
  fi
done
