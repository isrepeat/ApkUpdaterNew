# Google OAuth setup for ApkUpdaterNew

Create two Android OAuth clients in Google Cloud. The build uploader uses
a separate shared Desktop OAuth client and does not use these Android client IDs.

```text
Name: ApkUpdaterNew debug
Package name: com.isrepeat.apkupdaternew
SHA-1 certificate fingerprint: 5B:71:47:8C:04:C1:FF:D9:43:4A:4F:5E:F1:20:0D:2E:72:C1:B4:D5

Name: ApkUpdaterNew release
Package name: com.isrepeat.apkupdaternew
SHA-1 certificate fingerprint: 65:2C:85:FD:51:BD:AF:70:06:02:AB:1D:26:86:E4:FE:D4:6A:EE:0A
```

Google Cloud Clients page: https://console.cloud.google.com/auth/clients?project=androidappsstorage

This application's secrets are outside Git:

```text
C:\WORK\Secrets\Android\shared
├─ debug.keystore
├─ release.keystore
└─ signing.properties
```