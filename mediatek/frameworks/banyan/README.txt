MediaTek SDK
~~~~~~~~~~~~

MediaTek SDK is MediaTek developed SDK for Google Android. It allows developers
to write and test applications specifically for MediaTek platforms. The SDK
consists of two packages - "MediaTek SDK API" and "MediaTek SDK Toolset". The
SDK API package provides API libraries, documents, sample code and Android
add-on components. The toolset package offers MediaTek customized tools that
can help debugging.

Disclaimer: Code samples (under samples/ folder) have not been fully tested and
are hence provided for illustrative purposes only.


MediaTek SDK Libraries
~~~~~~~~~~~~~~~~~~~~~~

There are two API libraries in the SDK package, "mediatek-framework.jar" and
"mediatek-compatibility.jar. They are located under libs/ folder.


mediatek-framework.jar
~~~~~~~~~~~~~~~~~~~~~~

It is the MediaTek SDK API library that contains APIs of MediaTek platform specific
features. Applications can enhance their capabilities for MediaTek hardware by
leveraging from the APIs. For detailed usage, refer to the documentation included
in the SDK package.

It is to be noted that the library binary contains only API signatures. Do not
compile them directly into the application. Executing the code inside will cause
runtime exceptions.


mediatek-compatibility.jar
~~~~~~~~~~~~~~~~~~~~~~~~~~

It is an utility library that contains methods for checking if a feature is
supported by the device that the application is currently running on. When calling
those methods on non-MediaTek platforms, they will simply return negative values
(i.e. not supported).

To use it, copy the JAR file to your project's libs/ folder. In Eclipse, the library
classes should automatically be accessible.
