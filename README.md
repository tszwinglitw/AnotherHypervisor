# AnotherHypervisor

Logging my learning journey with Hypervisors and Virtualization. *just another hypervisor...*

#### References:
- [Hypervisor from Scratch](https://rayanfam.com/topics/hypervisor-from-scratch-part-1/)
- [CSE 291 - Virtualization](https://cseweb.ucsd.edu/~yiying/cse291-winter24/reading//)
	- [Memory Virtualization](https://cseweb.ucsd.edu/~yiying/cse291-winter24/reading/Virtualize-Memory.pdf)
- [Red Team Notes - Windows Kernel Internals](https://www.ired.team/miscellaneous-reversing-forensics/windows-kernel-internals/compiling-first-kernel-driver-kdprint-dbgprint-and-debugview)
- [5 Days to Virtualization: A Series on Hypervisor Development](https://revers.engineering/7-days-to-virtualization-a-series-on-hypervisor-development/)
---
#### Windows Host-Target Kernel Debugging
1. **Host Setup**
	- Ensure Windows Debugging Tools are installed on the host.
	- Locate `kdnet.exe` and `VerifiedNICList.xml` at: `C:\Program Files (x86)\Windows Kits\10\Debuggers\x64`
2. **Target Setup** ([ref](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/setting-up-a-network-debugging-connection-automatically))
	- Create a directory on the target: `C:\KDNET`
	- Copy `kdnet.exe` and `VerifiedNICList.xml` into `C:\KDNET` from host
	- Confirm that the output lists a supported NIC with `C:\KDNET>kdnet.exe`
3. **Enable Debugging**
	- Set the IP of the host and specify a unique debug port:
	- `C:\KDNET>kdnet.exe <HostComputerIPAddress> <YourDebugPort>`
	- Copy the generated key
4. **Connect to Target from Host**
	 - `WinDbgX.exe -k net:port=<DebugPort>,key=<GeneratedKey>`



##### Testing in VMware Workstation
- Enabling virtualization passthrough in VMware Workstation requires disabling all virtualization-based features on Windows, however [Windows Core Isolation/Memory Integrity](https://learn.microsoft.com/en-us/windows/security/hardware-security/enable-virtualization-based-protection-of-code-integrity?tabs=security) requires the hypervisor.
- Solution is to turn off Virtualization-based security (VBS) for enabling Nested Virtualization in VMware
	1. In Windows Features, turn off Hyper-V, Virtual Machine Platform and Windows Hypervisor Platform
	2. Turn off Core Isolation in Device Security
	- Alternatively, use the [Microsoft Device Guard and Credential Guard script](https://www.microsoft.com/en-us/download/details.aspx?id=53337) to disable and re-enable virtualization and virtualization-based security
	- Verify settings in `msinfo32`
		- Virtualization-based security should be off
		- No hypervisor should be detected
- Reference: 
	- [VMware Help Article](https://knowledge.broadcom.com/external/article/315385)  
	- [Options to optimize gaming performance in Windows 11](https://support.microsoft.com/en-us/windows/options-to-optimize-gaming-performance-in-windows-11-a255f612-2949-4373-a566-ff6f3f474613)
