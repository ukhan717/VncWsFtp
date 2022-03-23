ReadMe.txt for the ST STM32F746 SEGGER Eval Software

This project was built for SEGGER Embedded Studio V4.12.
It has been tested with the following versions:
- V4.12

The project STM32F746_ST_STM32F746G_DISCO_emWin_Simulation was built for Microsoft Studio 2010.
It has been tested with the following versions:
- Microsoft Visual Studio 2010
- Microsoft Visual Studio 2017 15.9.6

Supported hardware:
===================
The sample project for ST STM32F746NG is prepared
to run on a ST STM32F746G-DISCO, but may be used on other
target hardware as well.

Using different target hardware may require modifications.

Configurations:
===============
- Debug
  This configuration is prepared for download into
  internal Flash using J-Link.
  An embOS debug and profiling library is used.
  WARNING: Does not run stand-alone (see notes section)

- Release
  This configuration is prepared for download into
  internal Flash using J-Link.
  An embOS release library is used.

- Release_SystemView
  This configuration is prepared for download into
  internal Flash using J-Link.
  An embOS stack-check and profiling library is used.

Notes:
======
  To use SEGGER SystemView with Debug or Release_SystemView
  configuration, configure SystemViewer for STM32F746NG as
  target device and SWD at 4000 kHz as target interface.

  In order to use an application stand-alone (meaning with no debugger attached),
  either a release build must be used or the define SEGGER_RTT_MODE_DEFAULT must be changed from
  SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL to SEGGER_RTT_MODE_NO_BLOCK_SKIP in the file SEGGER_RTT_Conf.h

Included middleware components (object code versions):
================================================
- embOS V5.02a
  - (1.23.01.28) embOS-Cortex-M-SES (embOS designed for ARM Cortex-M and SEGGER Embedded Studio)

- emCompress V2.14
  - (19.00.00) emCompress (Compression system to reduce the storage requirements of data that must be embedded into an application)

- emCrypt V2.30
  - (12.50.04) emCrypt PRO
              (PLEASE NOTE: The emCrypt software is classified as dual-use good according to Category 5, Part 2 "Information Security" of
               EC Regulation No 428/2009. It is publicly available object code and can be downloaded from segger.com and therefore
               an export permission according to Art. 22 Abs. 8 und 10 EG-Dual-Use-VO is not required.)
  - (12.21.02) emCrypt STM32 Hardware Acceleration Add-On

- emFile V4.06a
  - (2.00.01) emFile FAT (FAT File system for embedded applications, supporting FAT12, FAT16 and FAT32)
  - (2.00.05) emFile Journaling (Journaling Add-on for emFile)
  - (2.03.00) embOS layer for emFile (abstraction layer for emFile providing OS specific functions)
  - (2.05.00) emFile Encryption (Encryption Add-on for emFile)
  - (2.10.10) emFile FAT LFN Module (Support for Long File Name)
  - Device drivers:
    - (2.10.03) emFile device driver for SD/SDHC/MultiMedia (Device driver for SD / SDHC & Multimedia card)
    - (2.10.04) emFile device driver for NOR flashes

- emWin V5.48o
  - (3.00.01) emWin BASE color (Complete graphic library, ANSI "C"-source code for 8/16/32 bit CPUs)
  - (3.00.04) emWin sim (emWin simulation)
  - (3.01.00) emWin WM/Widgets (Windows manager and GUIBuilder for emWin/GSC and Widgets)
  - (3.01.02) emWin Memory Devices (Memory devices for flicker-free animation)
  - (3.01.03) emWin Antialiasing (Antialiasing smoothes curves and diagonal lines)
  - (3.01.04) emWin VNC server (VNC server)
  - (3.02.01) emWin Bitmap Converter (Bitmap Converter for emWin)
  - (3.04.00) emWin Font Converter (Font Converter for emWin)
  - (3.xx.xx) embOS layer for emWin (Abstraction layer for emWin providing OS specific functions)
  - Drivers:
    - (3.10.23.02) GUIDRV_Lin

- IoT Toolkit V2.20
  - (15.00.01) IoT Toolkit

- embOS/IP V3.30c
  - (7.60.02) embOS/IP BASE IPv4/IPv6 Dual Stack (TCP/IP Protocol Stack including: IPv4, IPv6, ARP, ICMP, UDP, TCP, DHCPc, DNSc, ACD, Multicast, AutoIP, VLAN and BSD 4.4 Socket Interface, RAW sockets, TFTPc, TFTPs, Loopback device)
  - (7.02.00) embOS/IP FTP Server (File Transfer Protocol (Server))
  - (7.02.10) embOS/IP FTP Client (File Transfer Protocol (Client))
  - (7.03.01) embOS/IP NetBIOS Name Service (NetBIOS Name Service protocol)
  - (7.03.02) embOS/IP (m)DNS/LLMNR/DNS-SD Server(DNS protocol based servers)
  - (7.07.03) embOS/IP SNMP Agent Simple Network Management Protocol
  - (7.08.01) embOS/IP DHCP Server Dynamic Host Configuration Protocol (Server)
  - (7.08.03) embOS/IP UPnP Universal Plug&Play module
  - (7.14.00) embOS/IP SNTP Client Simple Network Time Protocol (Client)
  - (7.16.00) embOS/IP SMTP Client (Simple Mail Transfer Protocol (Client))
  - (7.19.00) embOS/IP MQTT Client MQ Telemetry Transport (MQTT) protocol (Client)
  - (7.22.00) embOS/IP CoAP Server/Client
  - (7.40.00) embOS/IP WebSocket WebSocket protocol
  - (7.30.00) embOS layer for embOS/IP (abstraction layer for embOS/IP providing OS specific functions)
  - Drivers:
    - (7.01.28) embOS/IP Synopsys (embOS/IP driver for CPUs with integrated Ethernet controller using Synopsys Ethernet IP)

- emWeb V3.30c
  - (7.05.00) embOS/IP Web Server (Hyper Text Transfer Protocol (Server))

- emModbus V1.02h
  - (14.00.00) emModbus Master (Modbus Master stack for embedded applications)
  - (14.00.01) emModbus Slave (Modbus Slave stack for embedded applications)
  - (14.xx.xx) embOS layer for emModbus (Abstraction layer for emModbus providing OS specific functions)

- emSecure RSA V2.40
  - (12.02.03.14) emSecure-RSA SP-SCL

- emSecure ECDSA V2.40
  - (12.02.13.14) emSecure-ECDSA SP-SCL

- emSSH V2.46
  - (15.10.00) emSSH (Secure Shell)
              (PLEASE NOTE: The emSSH software is classified as dual-use good according to Category 5, Part 2 "Information Security" of
               EC Regulation No 428/2009. It is publicly available object code and can be downloaded from segger.com and therefore
               an export permission according to Art. 22 Abs. 8 und 10 EG-Dual-Use-VO is not required.)
  - (15.11.02) emSSH STM32 Hardware Acceleration Add-On

- emSSL V2.56b
  - (7.11.00) emSSL (Secure Socket and Transport Layer Security Library)
             (PLEASE NOTE: The emSSL  software is classified as dual-use good according to Category 5, Part 2 "Information Security" of
              EC Regulation No 428/2009. It is publicly available object code and can be downloaded from segger.com and therefore
              an export permission according to Art. 22 Abs. 8 und 10 EG-Dual-Use-VO is not required.)
  - (7.11.04) emSSL STM32 Hardware Acceleration Add-On

- emUSB-Device V3.18
  - (9.00.00) emUSB-Device BASE (USB core + HID component)
  - (9.00.01) emUSB-Device Bulk Component (Bulk component + Windows driver (binary))
  - (9.00.03) emUSB-Device MSD Component (MSD component)
  - (9.00.04) emUSB-Device CDC Component (CDC component)
  - (9.00.05) emUSB-Device MSD-CDROM Component (MSD-CDROM component)
  - (9.00.07) emUSB-Device Printer Component (Printer Class component)
  - (9.00.10) emUSB-Device MTP Component (Media Transfer Protocol Component)
  - (9.00.11) emUSB-Device SmartMSD Component (Virtual mass storage device component)
  - (9.00.13) emUSB-Device Audio Component (Audio Protocol class)
  - (9.01.05) embOS layer for emUSB-Device (Abstraction layer for emUSB-Device providing OS specific functions)
  - (9.07.10) emUSB-Device-IP (IP-over-USB component to let USB devices be easily accessed with a web browser)
  - Drivers:
    - (9.10.56) emUSB-Device target driver ST STM32F2/F4/F7/H7 (Target driver for STM32F2, STM32F4, STM32F7, STM32H7 (FullSpeed / HighSpeed))

- emUSB-Host V2.18
  - (9.30.00) emUSB-Host BASE (emUSB Host Stack incl. MSD and HID)
  - (9.35.00) emUSB-Host Printer Class (emUSB-Host Printer Class)
  - (9.35.02) emUSB-Host CDC Class (emUSB-Host CDC class)
  - (9.35.03) emUSB-Host FTDI UART support (emUSB-Host FTDI UART support)
  - (9.35.04) emUSB-Host Bulk (emUSB-Host Bulk (vendor) class)
  - (9.35.06) emUSB-Host CCID (emUSB-Host CCID class)
  - (9.35.07) emUSB-Host MIDI (emUSB-Host MIDI class)
  - (9.35.08) emUSB-Host Audio (emUSB-Host Audio class)
  - (9.35.10) emUSB-Host MTP Class (emUSB-Host MTP class)
  - (9.37.10) emUSB-Host LAN (emUSB-Host LAN Component)
  - (9.60.00) embOS layer for emUSB-Host (Abstraction layer for emUSB-Host providing OS specific functions)
  - Drivers:
    - (9.40.05) emUSB-Host Synopsys DWC2 Full Speed driver (emUSB Host full speed driver for STM32F2/F4/F7, STM32F1 and XMC4500 host controllers)
    - (9.40.06) emUSB-Host Synopsys DWC2 High Speed driver (emUSB Host high speed driver for STM32F2/F4/F7/H7 host controllers)


