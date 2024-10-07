set PT=C:
set IMG=\VDB\vdb.wim
set sdi=\VDB\vdb.sdi
set ID={19260817-6666-8888-f00d-caffee000000}
bcdedit.exe /delete %ID%
bcdedit.exe /delete {ramdiskoptions} /f
bcdedit.exe /create {ramdiskoptions} /d "Wim Ramdisk"
bcdedit.exe /set {ramdiskoptions} ramdisksdidevice  partition=%PT%
bcdedit.exe /set {ramdiskoptions} ramdisksdipath %sdi%
bcdedit.exe -create %ID% /d "BootWim Once" /application osloader
bcdedit.exe -set %ID% device ramdisk="[%PT%]%IMG%,{ramdiskoptions}"
bcdedit.exe -set %ID% osdevice ramdisk="[%PT%]%IMG%,{ramdiskoptions}"
bcdedit.exe -set %ID% path \Windows\system32\boot\winload.efi
bcdedit.exe -set %ID% Locale zh-CN
bcdedit.exe -set %ID% systemroot \Windows
bcdedit.exe -set %ID% detecthal yes
bcdedit.exe -set %ID% winpe yes
bcdedit.exe -set %ID% pae forceenable
bcdedit.exe /bootsequence %ID% /addlast
echo OK
bcdedit.exe /enum %ID%
pause