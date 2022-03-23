REM 
REM Use gzip to compress static files
REM 
call gzip    ..\..\..\..\Tool\gzip html\css  bootstrap_min_css                    css
call gzip    ..\..\..\..\Tool\gzip html      Custom                               css
call gzip    ..\..\..\..\Tool\gzip html\js   bootstrap_min_js                     js
call gzip    ..\..\..\..\Tool\gzip html\js   events                               js
call gzip    ..\..\..\..\Tool\gzip html\js   jquery_min_js                        js
call gzip    ..\..\..\..\Tool\gzip html\js   tether_min_js                        js
call gzip    ..\..\..\..\Tool\gzip html\js   RGraphCC                             js
call gzip    ..\..\..\..\Tool\gzip html\js   RGraphCE                             js
call gzip    ..\..\..\..\Tool\gzip html\js   RGraphLi                             js
REM 
REM Convert the compressed files
REM Copy the .c/.h files into "Generated" folder
REM Delete the temp files from HTML folder
REM 
call cc_gzip ..\..\..\..\Tool\Bin2C html\css       bootstrap_min_css              gz
call cc_gzip ..\..\..\..\Tool\Bin2C html           Custom                         gz
call cc_gzip ..\..\..\..\Tool\Bin2C html\js        bootstrap_min_js               gz
call cc_gzip ..\..\..\..\Tool\Bin2C html\js        events                         gz
call cc_gzip ..\..\..\..\Tool\Bin2C html\js        jquery_min_js                  gz
call cc_gzip ..\..\..\..\Tool\Bin2C html\js        tether_min_js                  gz
call cc_gzip ..\..\..\..\Tool\Bin2C html\js        RGraphCC                       gz
call cc_gzip ..\..\..\..\Tool\Bin2C html\js        RGraphCE                       gz
call cc_gzip ..\..\..\..\Tool\Bin2C html\js        RGraphLi                       gz
REM 
REM Convert the non-static source code files, images, ...
REM Copy the .c/.h files into "Generated" folder
REM Delete the temp files from HTML folder
REM 
call cc      ..\..\..\..\Tool\Bin2C html           Authen                         htm
call cc      ..\..\..\..\Tool\Bin2C html           Error404                       htm
call cc      ..\..\..\..\Tool\Bin2C html           FormGET                        htm
call cc      ..\..\..\..\Tool\Bin2C html           FormPOST                       htm
call cc      ..\..\..\..\Tool\Bin2C html           index                          htm
call cc      ..\..\..\..\Tool\Bin2C html           IPConfig                       htm
call cc      ..\..\..\..\Tool\Bin2C html           OSInfo                         htm
call cc      ..\..\..\..\Tool\Bin2C html           Products                       htm
call cc      ..\..\..\..\Tool\Bin2C html           Samples                        htm
call cc      ..\..\..\..\Tool\Bin2C html           Shares                         htm
call cc      ..\..\..\..\Tool\Bin2C html           VirtFile                       htm
REM
REM ico files
REM
call cc      ..\..\..\..\Tool\Bin2C html           favicon                        ico
REM
REM png files
REM
call cc      ..\..\..\..\Tool\Bin2C html           emWeb_Logo                     png
REM
REM gif files
REM
call cc      ..\..\..\..\Tool\Bin2C html           embOSIP_Icon                   gif
call cc      ..\..\..\..\Tool\Bin2C html           embOSMPU_Icon                  gif
call cc      ..\..\..\..\Tool\Bin2C html           emFile_Icon                    gif
call cc      ..\..\..\..\Tool\Bin2C html           emLoad_Icon                    gif
call cc      ..\..\..\..\Tool\Bin2C html           Empty                          gif
call cc      ..\..\..\..\Tool\Bin2C html           emSSH_Icon                     gif
call cc      ..\..\..\..\Tool\Bin2C html           emSSL_Icon                     gif
call cc      ..\..\..\..\Tool\Bin2C html           emUSBD_Icon                    gif
call cc      ..\..\..\..\Tool\Bin2C html           emUSBH_Icon                    gif
call cc      ..\..\..\..\Tool\Bin2C html           emWin_Icon                     gif
call cc      ..\..\..\..\Tool\Bin2C html           GreenRUp                       gif
call cc      ..\..\..\..\Tool\Bin2C html           Logo                           gif
call cc      ..\..\..\..\Tool\Bin2C html           RedRDown                       gif
call cc      ..\..\..\..\Tool\Bin2C html           WhiteR                         gif

  
  