Public Class Form1

    Private FilePresent()() As Boolean
    Dim DirPaths As String() = New String() {"region\", "DIM-1\region\", "DIM1\region\"}
    Dim Names As String() = New String() {"r.0.0.mcr", "r.-1.0.mcr", "r.0.-1.mcr", "r.-1.-1.mcr"}
    Dim NamesA As String() = New String() {"r.0.0.mca", "r.-1.0.mca", "r.0.-1.mca", "r.-1.-1.mca"}
    Dim DimNames As String() = New String() {"Overworld", "Nether", "The End"}
    Dim StatusText As String()() = New String(2)() {New String(3) {"", "", "", ""}, New String(3) {"", "", "", ""}, New String(3) {"", "", "", ""}}
    Dim LevelDatPresent As Boolean = False
    Dim MinecraftSaveDir As String
    Dim AppDataPermissions As Boolean = True
    Dim DirInfo As IO.DirectoryInfo
    Dim PlayerFile As String, TargetDir As String

    ' This method calls a batch file or application and waits till it finishes running before continuing.
    Public Function ShellAndWait(ByVal ProcessPath As String, Optional ByVal CommandLine As String = "") As Boolean
        Dim objProcess As System.Diagnostics.Process
        Try

            ' Get info and run
            objProcess = New System.Diagnostics.Process()
            objProcess.StartInfo.FileName = ProcessPath
            objProcess.StartInfo.Arguments = CommandLine
            objProcess.StartInfo.WindowStyle = ProcessWindowStyle.Hidden
            objProcess.Start()

            'Wait until the process passes back an exit code 
            objProcess.WaitForExit()

            'Free resources associated with this process
            objProcess.Close()

            ' Return a success flag
            Return True

        Catch

            ' Return a failure flag
            Return False

        End Try

    End Function

    Private Sub ExitToolStripMenuItem_Click(sender As Object, e As EventArgs)
        Close()
    End Sub

    Private Sub OpenToolStripMenuItem1_Click(sender As Object, e As EventArgs) Handles OpenToolStripMenuItem1.Click

        ' Show the OpenFileDialog
        Dim Result As DialogResult = OpenFileDialog1.ShowDialog()

        ' If the user canceled, abort now
        If Result = Windows.Forms.DialogResult.Cancel Then Exit Sub

        ' If not, check to make sure the file exists and the name typed was not invalid
        If Result = Windows.Forms.DialogResult.OK And My.Computer.FileSystem.FileExists(OpenFileDialog1.FileName) Then

            ' Clear the directories if they exist
            If My.Computer.FileSystem.DirectoryExists(My.Application.Info.DirectoryPath + "\xbla\temp World") Then My.Computer.FileSystem.DeleteDirectory(My.Application.Info.DirectoryPath + "\xbla\temp World", FileIO.DeleteDirectoryOption.DeleteAllContents)
            If My.Computer.FileSystem.DirectoryExists(My.Application.Info.DirectoryPath + "\tools\temp World") Then My.Computer.FileSystem.DeleteDirectory(My.Application.Info.DirectoryPath + "\tools\temp World", FileIO.DeleteDirectoryOption.DeleteAllContents)

            ' Set the status text of the label
            lblFStatus.Text = "Extracting files..."
            Application.DoEvents()

            ' Copy the file to the working directory and try to extract
            My.Computer.FileSystem.CopyFile(OpenFileDialog1.FileName, My.Application.Info.DirectoryPath + "\xbla\savegame.dat", True)
            If ShellAndWait(My.Application.Info.DirectoryPath + "\xbla\extract.bat", "temp") = False Then
                MsgBox("Error: could not open the save game extractor.", MsgBoxStyle.Exclamation, "Extractor missing.")
                Exit Sub
            End If

            ' If here, the extractor finished, so now we examine the files it produced.
            ExamineXBoxWorld(My.Application.Info.DirectoryPath + "\xbla\temp World\")
            lblFStatus.Text = "savegame.dat open."

            ' Update all the information labels
            UInfo()
            Button1.Enabled = True
            SaveToolStripMenuItem.Enabled = True
            ToolStripMenuItem1.Enabled = True

            ' Show all the information labels
            For I As Integer = 1 To 11
                Controls.Find("Label" + Trim(Str(I)), True)(0).Show()
            Next
            Button1.Show()

            ' Move the directory to the processing center
            My.Computer.FileSystem.MoveDirectory(My.Application.Info.DirectoryPath + "\xbla\temp World", My.Application.Info.DirectoryPath + "\tools\temp World")

        Else

            ' Alert the user that the file was invalid
            MsgBox("'" + OpenFileDialog1.FileName + "' could not be found. Please double check your name and try again.", MsgBoxStyle.Exclamation, "File not found.")

        End If

    End Sub

    ' This looks through to see what region files exist in this XBox world
    Private Sub ExamineXBoxWorld(ByVal Directory As String)

        Dim D As IO.DirectoryInfo
	   AReset()

        ' For each dimension...
        For I As Integer = 0 To 2

            D = New IO.DirectoryInfo(Directory + DirPaths(I))

            ' Now look for each file in turn
            For Each F As IO.FileInfo In D.GetFiles()

                Select Case F.Name
                    Case "r.0.0.mcr"
                        FilePresent(I)(0) = True
                    Case "r.-1.0.mcr"
                        FilePresent(I)(1) = True
                    Case "r.0.-1.mcr"
                        FilePresent(I)(2) = True
                    Case "r.-1.-1.mcr"
                        FilePresent(I)(3) = True
                End Select

            Next

        Next

        ' Look for level.dat
        LevelDatPresent = My.Computer.FileSystem.FileExists(Directory + "\level.dat")

    End Sub

    ' Resets all labels to display files as not present
    Private Sub AReset()
        For I As Integer = 0 To 2
            For N As Integer = 0 To 3
                FilePresent(I)(N) = False
            Next
        Next
        LevelDatPresent = False
    End Sub

    ' Updates information on the labels
    Private Sub UInfo()
        For I As Integer = 0 To 2
            For N As Integer = 0 To 3
                If FilePresent(I)(N) Then
                    StatusText(I)(N) = "present"
                Else : StatusText(I)(N) = "not present"
                End If
            Next
        Next
        If LevelDatPresent Then
            Label11.Text = "present"
        Else : Label11.Text = "not present"
        End If
        UpdateLabels()
    End Sub

    ' Counts the number of files present
    Private Function ACount(ByVal Index As Integer) As Integer
        Dim Retval As Integer = 0
        For I As Integer = 0 To 3
            If FilePresent(Index)(I) Then Retval += 1
        Next
        Return Retval
    End Function

    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load

        ' Hide the report form
        Form2.Hide()

        ' Initialize file present array
        FilePresent = New Boolean(2)() {New Boolean(3) {False, False, False, False}, New Boolean(3) {False, False, False, False}, New Boolean(3) {False, False, False, False}}

        ' Check for application data
        If Not My.Computer.FileSystem.DirectoryExists(My.Computer.FileSystem.SpecialDirectories.CurrentUserApplicationData + "\xbox_to_pc_minecraft") Then
            Try
                My.Computer.FileSystem.CreateDirectory(My.Computer.FileSystem.SpecialDirectories.CurrentUserApplicationData + "\xbox_to_pc_minecraft")
            Catch ex As Exception
                MsgBox("Notice: this application does not have permission to save your settings. You will have to choose your Minecraft save directory every time you run the converter.", MsgBoxStyle.Exclamation, "No AppData permissions.")
                AppDataPermissions = False
            End Try
        Else

            ' Try to read path to Minecraft save directory
            If My.Computer.FileSystem.FileExists(My.Computer.FileSystem.SpecialDirectories.CurrentUserApplicationData + "\xbox_to_pc_minecraft\save_directory.dat") Then
                MinecraftSaveDir = My.Computer.FileSystem.ReadAllText(My.Computer.FileSystem.SpecialDirectories.CurrentUserApplicationData + "\xbox_to_pc_minecraft\save_directory.dat")
            End If

        End If

    End Sub

    Private Sub CustomizeToolStripMenuItem_Click(sender As Object, e As EventArgs) Handles CustomizeToolStripMenuItem.Click

        UpdateSaveDirectory()

    End Sub

    Private Sub UpdateSaveDirectory()

        ' Allow user to browse to the Minecraft save directory
        Dim Result As DialogResult = FolderBrowserDialog1.ShowDialog()

        ' If user canceled, abort now; otherwise, continue
        If Result = Windows.Forms.DialogResult.Cancel Then Exit Sub

        ' If here, set the save directory and try to save it to app data
        If My.Computer.FileSystem.DirectoryExists(FolderBrowserDialog1.SelectedPath) Then
            MinecraftSaveDir = FolderBrowserDialog1.SelectedPath
            My.Computer.FileSystem.WriteAllText(My.Computer.FileSystem.SpecialDirectories.CurrentUserApplicationData + "\xbox_to_pc_minecraft\save_directory.dat", MinecraftSaveDir, False)
        Else
            MsgBox("Could not find " + FolderBrowserDialog1.SelectedPath + ". Please check the folder name and try again.", MsgBoxStyle.Exclamation, "Folder not found.")
        End If

    End Sub

    Private Sub Convert(ByVal Preferences As Boolean)

        Dim L As Label, S As String, Nt As String

        If Form2 Is Nothing Then Form2 = New Form2
        Form2.TextBox1.Text = ""

        ' If there is no Minecraft save directory specified, get one now.
        If MinecraftSaveDir = "" Or Preferences Or Not My.Computer.FileSystem.DirectoryExists(MinecraftSaveDir) Then UpdateSaveDirectory()

        ' Deactivate button and menu
        Button1.Enabled = False
        ToolStripMenuItem1.Enabled = False
        SaveToolStripMenuItem.Enabled = False
        MenuStrip1.Enabled = False
        lblFStatus.Text = "Conversion in progress..."

        ' Set label information
        For I As Integer = 8 To 10
            L = Controls.Find("Label" + Trim(Str(I)), True)(0)
            For N As Integer = 0 To 3
                If FilePresent(I - 8)(N) Then
                    StatusText(I - 8)(N) = "waiting"
                Else : StatusText(I - 8)(N) = "not present"
                End If
            Next
            UpdateLabels()
        Next
        Label11.Text = "waiting"
        Application.DoEvents()

        ' Do the conversion
        For N As Integer = 0 To 2

            ' Add the dimention title to the report
            Form2.TextBox1.Text += DimNames(N) + vbCrLf + "----------------------------------" + vbCrLf + vbCrLf

            ' Nether flag for bedrock removal and End flag for proper biome setting
            Nt = ""
            If N = 1 Then Nt = " Nether"
            If N = 2 Then Nt = " End"

            ' For each file in this region...
            For I As Integer = 0 To 3

                ' If the file is present, convert it
                If FilePresent(N)(I) Then

                    If I = 0 Or I = 2 Then : S = "i " : Else : S = "- " : End If
                    If I = 0 Or I = 1 Then : S += "i" : Else : S += "-" : End If

                    ' Clean up from last one
                    StatusText(N)(I) = "preparing..."
                    UpdateLabels()
                    Application.DoEvents()
                    ShellAndWait(My.Application.Info.DirectoryPath + "\tools\delete.bat")

                    ' Extract XBox chunks
                    StatusText(N)(I) = "getting chunks..."
                    UpdateLabels()
                    Application.DoEvents()
                    ShellAndWait(My.Application.Info.DirectoryPath + "\tools\SeparateX.exe", """" + My.Application.Info.DirectoryPath + "\tools\temp World\" + DirPaths(N) + Names(I) + """ """ + My.Application.Info.DirectoryPath + "\tools\temp""")

                    ' Decompress XBox chunks
                    StatusText(N)(I) = "decompressing..."
                    UpdateLabels()
                    Application.DoEvents()
                    ShellAndWait(My.Application.Info.DirectoryPath + "\tools\decompress.bat")

                    ' Expand XBox chunks
                    StatusText(N)(I) = "converting chunks..."
                    UpdateLabels()
                    Application.DoEvents()
                    ShellAndWait(My.Application.Info.DirectoryPath + "\tools\ExpandX.exe", ("""" + My.Application.Info.DirectoryPath + "\tools\tempcompressed\"" """ + My.Application.Info.DirectoryPath + "\tools\temp\"" " + S + Nt).Replace("\", "/"))

                    ' Recompress chunks
                    StatusText(N)(I) = "recompressing..."
                    UpdateLabels()
                    Application.DoEvents()
                    ShellAndWait(My.Application.Info.DirectoryPath + "\tools\CompressP.exe", ("""" + My.Application.Info.DirectoryPath + "\tools\temp\"" """ + My.Application.Info.DirectoryPath + "\tools\tempcompressed\""").Replace("\", "/"))

                    ' Compile chunks
                    StatusText(N)(I) = "saving..."
                    UpdateLabels()
                    Application.DoEvents()
                    ShellAndWait(My.Application.Info.DirectoryPath + "\tools\ReconstructP.exe", ("""" + My.Application.Info.DirectoryPath + "\tools\tempcompressed\"" """ + My.Application.Info.DirectoryPath + "\tools\temp World\" + DirPaths(N) + NamesA(I) + """").Replace("\", "/"))

                    ' Done
                    StatusText(N)(I) = "complete"
                    UpdateLabels()
                    Application.DoEvents()

                    ' Add the log to the report
                    Form2.TextBox1.Text += My.Computer.FileSystem.ReadAllText(My.Application.Info.DirectoryPath + "\log.txt") + vbCrLf + "==================================" + vbCrLf + vbCrLf

                End If

            Next

        Next

        ' Keep getting world names until a valid one is given (world does not already exist)
        Do While TargetDir = "" Or My.Computer.FileSystem.DirectoryExists(MinecraftSaveDir + "\" + TargetDir + " World")
            TargetDir = InputBox("Please enter a name for your world:", "Name your world")
        Loop
        Form2.Label1.Text = "Conversion report - " + TargetDir

        ' Now convert level.dat
        ' Grab the first player file; that will be the source of the inventory
        My.Computer.FileSystem.CopyFile(My.Application.Info.DirectoryPath + "\tools\temp World\level.dat", My.Application.Info.DirectoryPath + "\tools\temp World\level.dat_xbox")
        DirInfo = New IO.DirectoryInfo(My.Application.Info.DirectoryPath + "\tools\temp World\players")
        If DirInfo.GetFiles().Length < 1 Then
            PlayerFile = "player_template.dat"
        Else
            PlayerFile = "temp World\players\" + DirInfo.GetFiles()(0).Name
        End If
        Label11.Text = "converting..."
        Application.DoEvents()
        ShellAndWait(My.Application.Info.DirectoryPath + "\tools\savegamedat.exe", ("""" + My.Application.Info.DirectoryPath + "\tools\level_template.dat"" """ + My.Application.Info.DirectoryPath + "\tools\temp World\level.dat"" """ + My.Application.Info.DirectoryPath + "\tools\" + PlayerFile + """ """ + TargetDir + """").Replace("\", "/"))

        ' Now compress level.dat
        Label11.Text = "compressing..."
        Application.DoEvents()
        ShellAndWait(My.Application.Info.DirectoryPath + "\tools\GZ_c.exe", ("""" + My.Application.Info.DirectoryPath + "\tools\temp World\"" level.dat").Replace("\", "/"))
        Label11.Text = "complete"

        ' Now move to the Minecraft save directory
        My.Computer.FileSystem.MoveDirectory(My.Application.Info.DirectoryPath + "\tools\temp World", MinecraftSaveDir + "\" + TargetDir + " World")
        lblFStatus.Text = "Conversion complete. Choose another file to convert."

        ' Reenable menu
        MenuStrip1.Enabled = True
        Button2.Enabled = True

    End Sub

    ' Update the text on the labels
    Private Sub UpdateLabels()

        Dim L As Label
        For I As Integer = 0 To 2
            L = Controls.Find("label" + Trim(Str(I + 8)), True)(0)
            L.Text = ""
            For N As Integer = 0 To 3
                L.Text += StatusText(I)(N) + vbCrLf
            Next
        Next

    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click

        Convert(False)

    End Sub

    Private Sub SaveToolStripMenuItem_Click(sender As Object, e As EventArgs) Handles SaveToolStripMenuItem.Click

        Convert(True)

    End Sub

    Private Sub ToolStripMenuItem1_Click(sender As Object, e As EventArgs) Handles ToolStripMenuItem1.Click

        Convert(False)

    End Sub

    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        Form2.Show()
    End Sub

End Class
