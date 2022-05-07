#include "stdafx.h"
#include "helper.hpp"
#include "external/inih/INIReader.h"
#include <string>

bool Proxy_Attach();
void Proxy_Detach();

using namespace std;

HMODULE baseModule = GetModuleHandle(NULL);

// INI Variables
bool bResFix;
bool bMovieFix;
bool bHUDFix;
int iFPSCap;
bool bFPSCap;
bool bFOVAdjust;
float fFOVAdjust;

// Variables
float fDesktopRight;
float fDesktopBottom;
float fDesktopAspect;
float fNativeAspect = 1.777777791f;
float fCustomAspect;
float fUIOffset;

// FPS Cap Hook
DWORD FPSCapReturnJMP;
float FPSCapValue;
void __declspec(naked) FPSCap_CC()
{
    __asm
    {
        divss xmm1, [FPSCapValue]
        movaps xmm0, xmm1
        jmp [FPSCapReturnJMP]
    }
}

// FOV1 Hook
DWORD FOV1ReturnJMP;
void __declspec(naked) FOV1_CC()
{
    __asm
    {
        movss [esp + 0x0C], xmm0
        movss xmm0, [eax + 0x3C]
        addss xmm0, [fFOVAdjust]
        jmp [FOV1ReturnJMP]
    }
}

// HUD Scale Hook
DWORD HudScaleReturnJMP;
float HudScaleValue;
void __declspec(naked) HudScale_CC()
{
    __asm
    {
        movss xmm0, [HudScaleValue]
        jmp[HudScaleReturnJMP]
    }
}

// HUD Left Offset Hook
DWORD HudLeftOffsetReturnJMP;
float HudLeftOffsetValue;
void __declspec(naked) HudLeftOffset_CC()
{
    __asm
    {
        subss xmm2, [HudLeftOffsetValue]
        jmp[HudLeftOffsetReturnJMP]
    }
}


void ReadConfig()
{
    INIReader config("RERevFix.ini");

    iFPSCap = config.GetInteger("FPS Cap", "Value", -1);
    bFPSCap = config.GetInteger("FPS Cap", "Enabled", true);
    bMovieFix = config.GetBoolean("Fix Movies", "Enabled", true);
    bResFix = config.GetBoolean("Custom Resolution", "Enabled", true);
    bHUDFix = config.GetBoolean("Fix UI", "Enabled", true);
    //iShadowQuality = config.GetInteger("Shadow Quality", "Value", -1);
    //bShadowQuality = config.GetBoolean("Shadow Quality", "Enabled", true);
    bFOVAdjust = config.GetBoolean("Increase FOV", "Enabled", true);
    fFOVAdjust = config.GetFloat("Increase FOV", "Value", -1);
    //iCustomResX = config.GetInteger("Custom Resolution", "Width", -1);
    //iCustomResY = config.GetInteger("Custom Resolution", "Height", -1);

    RECT desktop;
    GetWindowRect(GetDesktopWindow(), &desktop);
    fDesktopRight = (float)desktop.right;
    fDesktopBottom = (float)desktop.bottom;
    fDesktopAspect = (float)desktop.right / (float)desktop.bottom;
}

void Resolution()
{
    if (bResFix)
    { 
        // Address of signature = rerev.exe + 0x00751635
        // "\x85\xF6\x0F\x84\x00\x00\x00\x00\x81\xFE", "xxxx????xx"
        //uint8_t* CrashFixScanResult = Memory::PatternScan(baseModule, "85 F6 0F 84 ? ? ? ? 81 FE");
        //if (CrashFixScanResult) {
        //    Memory::PatchBytes((intptr_t)CrashFixScanResult, "\x90\x90", 2); // If you can't fix it, NOP it
        //    std::cout << "NOP'd crash area" << std::endl;
        //}

        //Address of signature = rerev.exe + 0x00CD3860
        //"\x31\x36\x3A\x39", "xxxx"
        uint8_t* SixteenNineScanResult = Memory::PatternScan(baseModule, "31 36 3A 39");
        if (SixteenNineScanResult) {
            Memory::PatchBytes((intptr_t)SixteenNineScanResult, "\x44\x45\x46\x41\x55\x4c\x54", 7); // Write DEFAULT to aspect       
            std::cout << "Wrote DEFAULT to aspect" << std::endl;
        }

        // Replace 640x480 with chosen res
        string newRes_string = std::to_string((int)fDesktopRight) + "x" + std::to_string((int)fDesktopBottom);
        uint8_t* TenEightyScanResult = Memory::PatternScan(baseModule, "36 34 30 78 34 38 30");
        if (TenEightyScanResult) {
            Memory::PatchBytes((intptr_t)TenEightyScanResult, newRes_string.c_str(), 9); // 3440x1440
            std::cout << "640x480 replaced with: " << newRes_string << std::endl;
        }

    }
}

void HUDFix()
{
    if (bHUDFix)
    {
        // Address of signature = rerev.exe + 0x00789572
        // "\xF3\x0F\x00\x00\x00\x00\x00\x00\x0F\x57\x00\xF3\x0F\x00\x00\xF3\x0F\x00\x00\xF3\x0F\x00\x00\x00\x00\x00\x00\x0F\x57\x00\xF3\x0F\x00\x00\xF3\x0F\x00\x00\x0F\x28", "xx??????xx?xx??xx??xx??????xx?xx??xx??xx"
        uint8_t* HudScaleScanResult = Memory::PatternScan(baseModule, "F3 0F ? ? ? ? ? ? 0F 57 ? F3 0F ? ? F3 0F ? ? F3 0F ? ? ? ? ? ? 0F 57 ? F3 0F ? ? F3 0F ? ? 0F 28");
        if (HudScaleScanResult) {
            int HudScaleHookLength = 8;
            DWORD HudScaleAddress = (intptr_t)HudScaleScanResult;
            HudScaleValue = (float)(fNativeAspect / fDesktopAspect) * 2;
            HudScaleReturnJMP = HudScaleAddress + HudScaleHookLength;
            Memory::Hook((void*)HudScaleAddress, HudScale_CC, HudScaleHookLength);
            std::cout << "Hud Scale set to: " << HudScaleValue << std::endl;
        }

        // Address of signature = rerev.exe + 0x007895A1
        // "\xF3\x0F\x00\x00\x00\x00\x00\x00\x0F\x28\x00\xF3\x0F\x00\x00\x00\x00\xF3\x0F\x00\x00\x00\x00\x00\x00\xF3\x0F\x00\x00\xF3\x0F\x00\x00\xF3\x0F\x00\x00\xF3\x0F\x00\x00\x00\xF3\x0F\x00\x00\x00\xF3\x0F\x00\x00\x00\x83\x66\x4C", "xx??????xx?xx????xx??????xx??xx??xx??xx???xx???xx???xxx"
        uint8_t* HudLeftOffsetScanResult = Memory::PatternScan(baseModule, "F3 0F ? ? ? ? ? ? 0F 28 ? F3 0F ? ? ? ? F3 0F ? ? ? ? ? ? F3 0F ? ? F3 0F ? ? F3 0F ? ? F3 0F ? ? ? F3 0F ? ? ? F3 0F ? ? ? 83 66 4C");
        if (HudLeftOffsetScanResult) {
            int HudLeftOffsetHookLength = 8;
            DWORD HudLeftOffsetAddress = (intptr_t)HudLeftOffsetScanResult;
            HudLeftOffsetValue = (float)fNativeAspect / fDesktopAspect;
            HudLeftOffsetReturnJMP = HudLeftOffsetAddress + HudLeftOffsetHookLength;
            Memory::Hook((void*)HudLeftOffsetAddress, HudLeftOffset_CC, HudLeftOffsetHookLength);
            std::cout << "Hud Left Offset set to: " << HudLeftOffsetValue << std::endl;
        }
    } 
}

void AdjustFOV()
{
    if (bFOVAdjust && fFOVAdjust > 0)
    {
        //Address of signature = rerev.exe + 0x0007246F
        // "\xF3\x0F\x00\x00\x00\x00\xF3\x0F\x00\x00\x00\xF3\x0F\x00\x00\x00\x00\x00\x00\xF3\x0F\x00\x00\x00\x00\xF3\x0F\x00\x00\x00\xF3\x0F\x00\x00\x00\x00\xF3\x0F\x00\x00\x00\xF3\x0F\x00\x00\x00\xE8", "xx????xx???xx??????xx????xx???xx????xx???xx???x"   
        uint8_t* FOV1ScanResult = Memory::PatternScan(baseModule, "F3 0F ? ? ? ? F3 0F ? ? ? F3 0F ? ? ? ? ? ? F3 0F ? ? ? ? F3 0F ? ? ? F3 0F ? ? ? ? F3 0F ? ? ? F3 0F ? ? ? E8");
        if (FOV1ScanResult)
        {
            int FOV1HookLength = 11;
            DWORD FOV1Address = (intptr_t)FOV1ScanResult;
            FOV1ReturnJMP = FOV1Address + FOV1HookLength;
            Memory::Hook((void*)FOV1Address, FOV1_CC, FOV1HookLength);

            #if _DEBUG
            std::cout << "FOV increased by " << (int)fFOVAdjust << std::endl;
            #endif	
        }
        else
        {
            #if _DEBUG
            std::cout << "FOV1 pattern scan failed." << std::endl;
            #endif	
            return;
        }
    }
    
}

void FPSCap()
{
    if (bFPSCap && iFPSCap == 0) // Don't leave it at 0, assume they want it "uncapped".
    {
        iFPSCap = 999;
    }

    if (bFPSCap && iFPSCap > 120)
    {
        // Address of signature = rerev.exe + 0x005CC311
        // "\xF3\x0F\x00\x00\x00\x0F\x28\x00\xF3\x0F\x00\x00\xF3\x0F\x00\x00\x00\x00\xD9\x54", "xx???xx?xx??xx????xx"  
        uint8_t* FPSCapScanResult = Memory::PatternScan(baseModule, "F3 0F ? ? ? 0F 28 ? F3 0F ? ? F3 0F ? ? ? ? D9 54");

        if (FPSCapScanResult)
        {
            int FPSCapHookLength = 8;
            DWORD FPSCapAddress = (intptr_t)FPSCapScanResult;
            FPSCapValue = (float)iFPSCap;
            FPSCapReturnJMP = FPSCapAddress + FPSCapHookLength;
            Memory::Hook((void*)FPSCapAddress, FPSCap_CC, FPSCapHookLength);

            #if _DEBUG
            std::cout << "FPS Cap set to: " << (int)iFPSCap << std::endl;
            #endif	
        }
        else
        {
            #if _DEBUG
            std::cout << "FPSCap pattern scan failed." << std::endl;
            #endif	
            return;
        }
    }
}

DWORD __stdcall Main(void*)
{
    Sleep(400); // delay first

    #if _DEBUG
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    std::cout << "Console initiated" << std::endl;
    #endif	
    ReadConfig();
    Resolution();
    HUDFix();
    AdjustFOV();
    FPSCap();

    return true; // end thread
}

HMODULE ourModule; 

void Patch_Uninit()
{

}

BOOL APIENTRY DllMain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        ourModule = hModule;
        Proxy_Attach();

        CreateThread(NULL, 0, Main, 0, NULL, 0);
    }
    if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        Patch_Uninit();

        Proxy_Detach();
    }

    return TRUE;
}
