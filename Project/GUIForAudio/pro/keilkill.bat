FOR /F "tokens=*" %%G IN ('DIR /B /S *.bak') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.ddk') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.edk') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.lst') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.lnp') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.mpf') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.mpj') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.obj') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.omf') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.plg') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.rpt') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.tmp') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.__i') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.crf') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.o') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.d') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.axf') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.bin') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.hex') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.tra') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.dep') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.iex') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.htm') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.sct') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S *.map') DO echo "%%G" && del /Q "%%G"
FOR /F "tokens=*" %%G IN ('DIR /B /S JLinkLog.txt') DO echo "%%G" && del /Q "%%G"

