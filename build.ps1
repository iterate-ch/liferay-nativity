Param(
    [Switch] $skipBuild,
    [Switch] $skipInstall,
	[String] $version,
    [Switch] $debugBuild
)

if (-not $version) {
    $pom = [xml](Get-Content pom.xml)
    $version = [String]$pom.project.version
}

Write-Host "** Building for ${version} **"

$configuration = "";
if ($debugBuild) {
    $configuration = "-Dmsbuild.configuration=Debug"
}
else {
    $configuration = "-Dmsbuild.configuration=Release"
}

if (-not $skipBuild) {
    mvn clean install
    ant -f build-overlays.xml overlays $configuration
    ant -f build.xml build-windows-menus $configuration
}

if (-not $skipInstall) {
    foreach ($file in Get-ChildItem dist\*.dll) {
        Write-Host "Adding $($file.BaseName)"
        mvn install:install-file `
            -Dfile="$($file.FullName)" `
            -DgroupId="ch.iterate.liferay" `
            -DartifactId="$($file.BaseName)" `
            -Dversion="${version}" `
            -Dpackaging="dll" -Dclassifier="native" -DgeneratePom="true"
    }
}