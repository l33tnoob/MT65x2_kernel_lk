
----- porting -------

1.create an empty project called HelloAndroid
2.add two buttons in activity_main.xml (one for "RemoteStart" and another for "RemoteBinding" test case)
3.add OnClickListener() of above buttons in MainActivity.java
  * OnClickListener() will be used to handle all different test cases for convenience 
4.add components to HelloAndroid Manifest from ApiDemo Manifest
  <service android:name=".RemoteService" android:process=":remote">
  <activity android:name=".RemoteService$Controller" android:launchMode="singleTop">
  <activity android:name=".RemoteService$Binding">   
  * note that package name should be "com.example.helloandroid"
  * above two activities will be invoked in OnClickListener() in MainActivity via Intent 
5.import code to HelloAndroid source directory from ApiDemo 
  RemoteService.java
  ISecondary.aidl
  IRemoteService.aidl
  IRemoteServiceCallback.aidl
  * aidl files will be automatically generated to java files in the gen directory
  * in real case, client and server side all need to have the AIDL file 
6.solve compile error
  - modify package name
  - add resource (layout / string / drawable) / copy from ApiDemo

