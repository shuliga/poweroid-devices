Get-ChildItem ..\..\cmake-build-release\* -Include '*.hex' | foreach { (Get-FileHash $_).Hash > "$($_.Name).sha256" }