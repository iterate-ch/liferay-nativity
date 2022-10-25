Param(
	[Parameter(Mandatory=$true)] [String] $version
)

mvn clean install
ant -f build-overlays.xml overlays
ant -f build.xml build-windows-menus

foreach ($file in Get-ChildItem dist\*.dll) {
    Write-Host "Adding $($file.BaseName)"
    mvn install:install-file `
        -Dfile="$($file.FullName)" `
        -DgroupId="ch.iterate.liferay" `
        -DartifactId="$($file.BaseName)" `
        -Dversion="$($version)" `
        -Dpackaging="dll" -Dclassifier="native" -DgeneratePom="true"
}
