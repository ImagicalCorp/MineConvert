:: SeparateX %1 ../temp
cd tools
FOR /f %%F IN ('DIR /b .\temp\*.dat') DO quickbms xmem2.bms ./temp/%%F ./tempcompressed
:: ExpandX ../tempcompressed/ ../temp/ %2 %3%4
:: CompressP ../temp/ ../tempcompressed/
:: ReconstructP ../tempcompressed/ %5
:: FOR /f %%F IN ('DIR /b ..\temp\*.dat') DO DEL ..\temp\%%F
:: FOR /f %%F IN ('DIR /b ..\tempcompressed\*.dat') DO DEL ..\tempcompressed\%%F