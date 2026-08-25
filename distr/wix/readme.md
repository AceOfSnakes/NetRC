# Install wix
```
dotnet tool install --global wix
wix extension add -acceptEula wix7 WixToolset.UI.wixext --global
```
# Make msi archive
```
wix build -acceptEula wix7 app.wxs -ext WixToolset.UI.wixext -arch x64 -o netrc_26.08_amd64.msi
```