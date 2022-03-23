REM
REM Convert the source code files, images, ...
REM Copy the .c/.h files into "Generated" folder
REM Delete the temp files from HTML folder
REM
call cc ..\..\..\..\Tool\Bin2C html\css       bootstrap_min_css              css
call cc ..\..\..\..\Tool\Bin2C html           Custom                         css
call cc ..\..\..\..\Tool\Bin2C html\js        bootstrap_min_js               js
call cc ..\..\..\..\Tool\Bin2C html\js        events                         js
call cc ..\..\..\..\Tool\Bin2C html\js        jquery_min_js                  js
call cc ..\..\..\..\Tool\Bin2C html\js        tether_min_js                  js
call cc ..\..\..\..\Tool\Bin2C html\js        RGraphCC                       js
call cc ..\..\..\..\Tool\Bin2C html\js        RGraphCE                       js
call cc ..\..\..\..\Tool\Bin2C html\js        RGraphLi                       js
call cc ..\..\..\..\Tool\Bin2C html           index                          htm
call cc ..\..\..\..\Tool\Bin2C html           VirtFile                       htm
call cc ..\..\..\..\Tool\Bin2C html           Samples                        htm
call cc ..\..\..\..\Tool\Bin2C html           Shares                         htm
call cc ..\..\..\..\Tool\Bin2C html           OSInfo                         htm
call cc ..\..\..\..\Tool\Bin2C html           FormGET                        htm
call cc ..\..\..\..\Tool\Bin2C html           FormPOST                       htm
call cc ..\..\..\..\Tool\Bin2C html           IPConfig                       htm
call cc ..\..\..\..\Tool\Bin2C html           Authen                         htm
call cc ..\..\..\..\Tool\Bin2C html           Products                       htm
call cc ..\..\..\..\Tool\Bin2C html           emWeb_Logo                     png
call cc ..\..\..\..\Tool\Bin2C html           Logo                           gif
call cc ..\..\..\..\Tool\Bin2C html           GreenRUp                       gif
call cc ..\..\..\..\Tool\Bin2C html           RedRDown                       gif
call cc ..\..\..\..\Tool\Bin2C html           WhiteR                         gif
call cc ..\..\..\..\Tool\Bin2C html           favicon                        ico
call cc ..\..\..\..\Tool\Bin2C html           Empty                          gif
call cc ..\..\..\..\Tool\Bin2C html           embOSMPU_Icon                  gif
call cc ..\..\..\..\Tool\Bin2C html           emFile_Icon                    gif
call cc ..\..\..\..\Tool\Bin2C html           emLoad_Icon                    gif
call cc ..\..\..\..\Tool\Bin2C html           embOSIP_Icon                   gif
call cc ..\..\..\..\Tool\Bin2C html           emSSH_Icon                     gif
call cc ..\..\..\..\Tool\Bin2C html           emSSL_Icon                     gif
call cc ..\..\..\..\Tool\Bin2C html           emUSBD_Icon                    gif
call cc ..\..\..\..\Tool\Bin2C html           emUSBH_Icon                    gif
call cc ..\..\..\..\Tool\Bin2C html           emWin_Icon                     gif