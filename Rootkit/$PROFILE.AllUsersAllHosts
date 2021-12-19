function Install-Driver($name)
{
	$cleanName = $name -replace ".sys|.\\", ""

	sc.exe stop $cleanName
	sc.exe delete $cleanName

	cp $name c:\windows\system32\drivers\ -verbose -force
	sc.exe create $cleanName type= kernel start= demand error= normal binPath= c:\windows\System32\Drivers\$cleanName.sys DisplayName= $cleanName

	sc.exe start $cleanName
}