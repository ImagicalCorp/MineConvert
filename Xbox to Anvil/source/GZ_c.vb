Module Module1

    Sub Main()

        Using originalFileStream As IO.Stream = New IO.StreamReader(My.Application.CommandLineArgs(0) + My.Application.CommandLineArgs(1)).BaseStream
            Using compressedFileStream As IO.FileStream = IO.File.Create(My.Application.CommandLineArgs(0) + "level.dat_gz")
                Using compressionStream As IO.Compression.GZipStream = New IO.Compression.GZipStream(compressedFileStream, IO.Compression.CompressionLevel.Fastest)
                    originalFileStream.CopyTo(compressionStream)
                End Using
            End Using
        End Using

        My.Computer.FileSystem.DeleteFile(My.Application.CommandLineArgs(0) + My.Application.CommandLineArgs(1))
        My.Computer.FileSystem.RenameFile(My.Application.CommandLineArgs(0) + "level.dat_gz", "level.dat")

    End Sub

End Module
