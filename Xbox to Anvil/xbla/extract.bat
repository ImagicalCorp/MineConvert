:: Copyright of John Conn (Rocky5 Forums & JCRocky5 Twitter) 2013
:: Please dont re-release this as your own, if you make a better tool then I dont mind :-)

@Echo off
cd xbla

::Errors
if not exist "savegame.dat" CLS & Echo: & Echo  Savegame.dat is missing. & Echo: & Pause  & Exit
if not exist "Tools" CLS & Echo: & Echo  Tools folder is missing. & Echo: & Pause  & Exit
if not exist "Tools\sfk.exe" CLS & Echo: & Echo  Tools\skf.exe is missing. & Echo: & Pause  & Exit

:: Hide output information (on by default)
Set "Hideoutput=true"

:: Tools
Set "SFK=Tools\sfk.exe"
Set "MineTool=Tools\minecraft.exe"

:: Decompressed file name
Set "file=savegame.decompressed"

:: Index offsets
Set "Indexoffset=0x0 0x04"
Set "Indexsize=0x04 0x04"

:: MCR offsets
Set "Indexmcroffset=0x84 0x04"
Set "Indexmcrsize=0x80 0x04"
Set "Mcrnameoffset=0x0 0x38"
Set "Profilenameoffset=0x0 0x10"

:: Index files are 144 bytes long
Set "Indexfilesbytelength=144"

:: File names
Set "Level=006C006500760065006C002E0064006100740000000000000000000000000000000000000000000000000000000000000000000000000000"
Set "r00mcr=0072002E0030002E0030002E006D006300720000000000000000000000000000000000000000000000000000000000000000000000000000"
Set "r0-1mcr=0072002E0030002E002D0031002E006D00630072000000000000000000000000000000000000000000000000000000000000000000000000"
Set "r-00mcr=0072002E002D0031002E0030002E006D00630072000000000000000000000000000000000000000000000000000000000000000000000000"
Set "r-1-1mcr=0072002E002D0031002E002D0031002E006D0063007200000000000000000000000000000000000000000000000000000000000000000000"
Set "dim1r0-1mcr=00440049004D0031002F0072002E0030002E002D0031002E006D006300720000000000000000000000000000000000000000000000000000"
Set "dim1r-10mcr=00440049004D0031002F0072002E002D0031002E0030002E006D006300720000000000000000000000000000000000000000000000000000"
Set "dim1r-1-1mcr=00440049004D0031002F0072002E002D0031002E002D0031002E006D00630072000000000000000000000000000000000000000000000000"
Set "dim1r00mcr=00440049004D0031002F0072002E0030002E0030002E006D0063007200000000000000000000000000000000000000000000000000000000"
Set "dim-1r00mcr=00440049004D002D00310072002E0030002E0030002E006D0063007200000000000000000000000000000000000000000000000000000000"
Set "dim-1r0-1mcr=00440049004D002D00310072002E0030002E002D0031002E006D006300720000000000000000000000000000000000000000000000000000"
Set "dim-1r-10mcr=00440049004D002D00310072002E002D0031002E0030002E006D006300720000000000000000000000000000000000000000000000000000"
Set "dim-1r-1-1mcr=00440049004D002D00310072002E002D0031002E002D0031002E006D00630072000000000000000000000000000000000000000000000000"
Set "map0dat=0064006100740061002F006D00610070005F0030002E00640061007400000000000000000000000000000000000000000000000000000000"
Set "map1dat=0064006100740061002F006D00610070005F0031002E00640061007400000000000000000000000000000000000000000000000000000000"
Set "map2dat=0064006100740061002F006D00610070005F0032002E00640061007400000000000000000000000000000000000000000000000000000000"
Set "map3dat=0064006100740061002F006D00610070005F0033002E00640061007400000000000000000000000000000000000000000000000000000000"
Set "map4dat=0064006100740061002F006D00610070005F0034002E00640061007400000000000000000000000000000000000000000000000000000000"
Set "map5dat=0064006100740061002F006D00610070005F0035002E00640061007400000000000000000000000000000000000000000000000000000000"
Set "map6dat=0064006100740061002F006D00610070005F0036002E00640061007400000000000000000000000000000000000000000000000000000000"
Set "map7dat=0064006100740061002F006D00610070005F0037002E00640061007400000000000000000000000000000000000000000000000000000000"
Set "map8dat=0064006100740061002F006D00610070005F0038002E00640061007400000000000000000000000000000000000000000000000000000000"
Set "map9dat=0064006100740061002F006D00610070005F0039002E00640061007400000000000000000000000000000000000000000000000000000000"
Set "mapmapping=0064006100740061002F006D006100700044006100740061004D0061007000700069006E00670073002E0064006100740000000000000000"
Set "players=0070006C00610079006500720073002F"

: Give credit to Rocky5
Echo Mincraft XBLA World Extractor
Echo Thanks to Rocky5
Echo

:Prep

:: Reset var
Set Outputfolder=

:: Simple GUI

Set "Outputfolder=%1"
if "%Outputfolder%"=="" Exit
if exist "%Outputfolder% World" Echo Duplicate folder, change. & timeout /t 2  & Goto Prep

@Echo on

rd /s /q tmp 2
if not exist %file% "%MineTool%" -d savegame.dat %file%  

:Start

:: Create folder structure
md "tmp\mcr_extracted\region"
md "tmp\mcr_extracted\DIM1\region"
md "tmp\mcr_extracted\DIM-1\region"
md "tmp\mcr_extracted\players"
md "tmp\mcr_extracted\data\map"

:: Dump Data needed
%SFK% hexdump -nofile -pure -offlen %Indexoffset% %file%>>"tmp\Indexoffset"
%SFK% hexdump -nofile -pure -offlen %Indexsize% %file%>>"tmp\Indexsize"

:: Read & Export Index
For /f "eol=. tokens=1" %%a in (tmp\Indexoffset) do Set "Indexoffset=%%a"
For /f "eol=. tokens=1" %%b in (tmp\Indexsize) do Set "Indexsize=%%b"

%SFK% dec %Indexoffset%>>"tmp\OffsetDec"
%SFK% dec %Indexsize%>>"tmp\SizeDec"

For /f "eol=. tokens=1" %%a in (tmp\OffsetDec) do Set "OffsetDec=%%a"
For /f "eol=. tokens=1" %%b in (tmp\SizeDec) do Set "SizeDec=%%b"

:: Calculate Index size
set /a "SizeDecTotal=%SizeDec%*%Indexfilesbytelength%"

:: Show sizes, was for me.
::Echo %SizeDec% ^* %Indexfilesbytelength% ^= %SizeDecTotal%

:: Dump Index
Echo >>tmp\Index
%SFK% partcopy "%file%" %OffsetDec% %SizeDecTotal% tmp\Index 0 -yes

::Fix Byte Size
Set /a Size=0

:Indexfiles

:: Used to clear the screen for a better looking verbose
CLS & Echo: & Echo: & Echo:

:: Add 1 each loop
Set /a Count+=1
ECHO Index file count ^= !Count!

:: Create mcr* files
Echo a>>tmp\mcr%Count%
Echo a>>tmp\mcr_extracted\mcr%Count%
%SFK% partcopy "tmp\Index" %Size% %Indexfilesbytelength% tmp\mcr%Count% 0 -yes

:MCRfiles

:: Get offsets & sizes
%SFK% hexdump -nofile -pure -offlen %Indexmcroffset% "tmp\mcr%Count%">>"tmp\Indexmcroffset%Count%"
%SFK% hexdump -nofile -pure -offlen %Indexmcrsize% "tmp\mcr%Count%">>"tmp\Indexmcrsize%Count%"

:: Read & Export Index
For /f "eol=. tokens=1" %%a in (tmp\Indexmcroffset%Count%) do Set "Indexmcroffset2=%%a"
For /f "eol=. tokens=1" %%b in (tmp\Indexmcrsize%Count%) do Set "Indexmcrsize2=%%b"
%SFK% dec %Indexmcroffset2%>>"tmp\IndexmcroffsetDec%Count%"
%SFK% dec %Indexmcrsize2%>>"tmp\IndexmcrsizeDec%Count%"
For /f "eol=. tokens=1" %%a in (tmp\IndexmcroffsetDec%Count%) do Set "IndexmcroffsetDec=%%a"
For /f "eol=. tokens=1" %%b in (tmp\IndexmcrsizeDec%Count%) do Set "IndexmcrsizeDec=%%b"
::Echo %IndexmcroffsetDec%
::Echo %IndexmcrsizeDec%
%SFK% partcopy %file% %IndexmcroffsetDec% %IndexmcrsizeDec% tmp\mcr_extracted\mcr%Count% 0 -yes

:: Rename files to there corrent names.
%SFK% hexdump -nofile -recsize 80 -pure -offlen %Mcrnameoffset% tmp\mcr%Count%>>"tmp\mcrname"
%SFK% hexdump -nofile -recsize 80 -pure -offlen %Profilenameoffset% tmp\mcr%Count%>>"tmp\mcrplayer"
For /f "tokens=*" %%a in (tmp\mcrname) do Set "filename=%%a"
For /f "tokens=*" %%a in (tmp\mcrplayer) do Set "filename2=%%a"

:: level.dat
if "%Level%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "level.dat"

:: Region files
if "%r00mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.0.0.mcr" & move "tmp\mcr_extracted\r.0.0.mcr" "tmp\mcr_extracted\region"
if "%r0-1mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.0.-1.mcr" & move "tmp\mcr_extracted\r.0.-1.mcr" "tmp\mcr_extracted\region"
if "%r-00mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.-1.0.mcr" & move "tmp\mcr_extracted\r.-1.0.mcr" "tmp\mcr_extracted\region"
if "%r-1-1mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.-1.-1.mcr" & move "tmp\mcr_extracted\r.-1.-1.mcr" "tmp\mcr_extracted\region"

:: Nether Region files
if "%dim1r00mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.0.0.mcr" & move "tmp\mcr_extracted\r.0.0.mcr" "tmp\mcr_extracted\DIM1\region"
if "%dim1r0-1mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.0.-1.mcr" & move "tmp\mcr_extracted\r.0.-1.mcr" "tmp\mcr_extracted\DIM1\region"
if "%dim1r-10mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.-1.0.mcr" & move "tmp\mcr_extracted\r.-1.0.mcr" "tmp\mcr_extracted\DIM1\region"
if "%dim1r-1-1mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.-1.-1.mcr" & move "tmp\mcr_extracted\r.-1.-1.mcr" "tmp\mcr_extracted\DIM1\region"

:: End Region files
if "%dim-1r00mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.0.0.mcr" & move "tmp\mcr_extracted\r.0.0.mcr" "tmp\mcr_extracted\DIM-1\region"
if "%dim-1r0-1mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.0.-1.mcr" & move "tmp\mcr_extracted\r.0.-1.mcr" "tmp\mcr_extracted\DIM-1\region"
if "%dim-1r-10mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.-1.0.mcr" & move "tmp\mcr_extracted\r.-1.0.mcr" "tmp\mcr_extracted\DIM-1\region"
if "%dim-1r-1-1mcr%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "r.-1.-1.mcr" & move "tmp\mcr_extracted\r.-1.-1.mcr" "tmp\mcr_extracted\DIM-1\region"

:: Data files
if "%map0dat%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "map_0.dat" & move "tmp\mcr_extracted\map_0.dat" "tmp\mcr_extracted\data"
if "%map1dat%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "map_1.dat" & move "tmp\mcr_extracted\map_1.dat" "tmp\mcr_extracted\data"
if "%map2dat%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "map_2.dat" & move "tmp\mcr_extracted\map_2.dat" "tmp\mcr_extracted\data"
if "%map3dat%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "map_3.dat" & move "tmp\mcr_extracted\map_3.dat" "tmp\mcr_extracted\data"
if "%map4dat%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "map_4.dat" & move "tmp\mcr_extracted\map_4.dat" "tmp\mcr_extracted\data"
if "%map5dat%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "map_5.dat" & move "tmp\mcr_extracted\map_5.dat" "tmp\mcr_extracted\data"
if "%map6dat%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "map_6.dat" & move "tmp\mcr_extracted\map_6.dat" "tmp\mcr_extracted\data"
if "%map7dat%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "map_7.dat" & move "tmp\mcr_extracted\map_7.dat" "tmp\mcr_extracted\data"
if "%map8dat%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "map_8.dat" & move "tmp\mcr_extracted\map_8.dat" "tmp\mcr_extracted\data"
if "%map9dat%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "map_9.dat" & move "tmp\mcr_extracted\map_9.dat" "tmp\mcr_extracted\data"
if "%mapmapping%"=="%filename%" ren "tmp\mcr_extracted\mcr%Count%" "DataMappings.dat" & move "tmp\mcr_extracted\DataMappings.dat" "tmp\mcr_extracted\data\map"

:: Player files
if "%players%"=="%filename2%" (
%SFK% head "tmp\mcr%Count%">>tmp\playername%Count%
move "tmp\mcr_extracted\mcr%Count%" "tmp\mcr_extracted\players\mcr%Count%"
For /f "tokens=*" %%g in (tmp\playername%Count%) do Set "new=%%g"
SET "result=!new:.=!"
ren "tmp\mcr_extracted\players\mcr%Count%" "!result:~8,-14!.dat"
)

:: Cleanup
del /q "tmp\mcrname"
del /q "tmp\mcrplayer"

:: Add 144 to each loop
Set /a Size+=144

:: Loop if file count doesnt match counter
if not "%SizeDec%"=="%Count%" Goto Indexfiles


:Finish

:: Move extracted folder
Move "tmp\mcr_extracted" "%CD%"
Ren "%CD%\mcr_extracted" "%Outputfolder% World"

:: Remove tmp folder & decompressed file
rd /s /q tmp 
del /q %file% 

:: Copyright of John Conn (Rocky5 Forums & JCRocky5 Twitter) 2013