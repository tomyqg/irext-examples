# irext Android decode Java wrapper
Decode the IR binary file into IR time-series offline according to your key input

#### Usage
1. Enter the directory of '/decodesdk/irextdecode' and compile with ndk-build to generate shared libraries  

2. Copy all the shared library files (.so) to the correct path of your Android project  

3. Copy the decodesdk project to your Android APP project and make it as a module in Android Studio  

4. Call the following methods to help you decoding the downloaded IR binaries  

Import decoder class:

    import net.irext.decodesdk.bean.*;
    
    import net.irext.decodesdk.utils.Constants;
    
    import net.irext.decodesdk.IRDecode;
    

Get decoder instance:

    IRDecode irDecoder = IRDecode.getInstance();
    
Load AC binary file:

    irDecode.openACBinary(binFileName);

Load TV binary file:

    irDecode.openTVBinary(binFileName, subCategory);
    // subCategory is 'sub_cate' property of your downloaded remote index

Decode AC control command:

    irDecode.decodeACBinary(acStatus, acFunction);
    // ACStatus refers to the current state of Air conditioner, please check ACStatus.java for information

Decode TV control command:

    irDecode.decodeTVBinary(keyCode)
    // For key definition, please refer to http://irext.net/doc/index.html
    
Close AC binary file:

    irDecode.closeACBinary();
    
Close TV binary file:

    irDecode.closeTVBinary();
    
Get AC supported modes:

    int []supportedModes = irDecode.getACSupportedMode();
    
Get AC supported temperature range in a certain mode:

    TemperatureRange tr = irDecode.getTemperatureRange(mode);
    
Get AC supported wind speed levels in a certain mode:

    int []windLevels = irDecode.getACSupportedWindSpeed(mode);
    
Get AC supported wind directions in a certain mode:

    int []windDirs = irDecode.getACSupportedSwing();

