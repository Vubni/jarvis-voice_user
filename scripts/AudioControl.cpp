#include "audioControl.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <vector>
#include <algorithm>

struct SessionVolume {
    DWORD pid;
    float volume;
};

std::vector<SessionVolume> savedVolumes;

void MuteOtherApplications(float reductionPercent) {
    if (!savedVolumes.empty()) {
        return; // Уже приглушено
    }

    HRESULT hr;
    bool coInitialized = false;

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioSessionManager2* pSessionManager = nullptr;
    IAudioSessionEnumerator* pSessionEnumerator = nullptr;
    int sessionCount = 0;
    DWORD currentPID = 0;

    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        if (hr != RPC_E_CHANGED_MODE) {
            return;
        }
        coInitialized = false;
    } else {
        coInitialized = true;
    }

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) goto cleanup;

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) goto cleanup;

    hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL,
                           nullptr, (void**)&pSessionManager);
    if (FAILED(hr)) goto cleanup;

    hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
    if (FAILED(hr)) goto cleanup;

    hr = pSessionEnumerator->GetCount(&sessionCount);
    if (FAILED(hr)) goto cleanup;

    currentPID = GetCurrentProcessId();

    for (int i = 0; i < sessionCount; ++i) {
        IAudioSessionControl* pSessionControl = nullptr;
        hr = pSessionEnumerator->GetSession(i, &pSessionControl);
        if (FAILED(hr)) continue;

        IAudioSessionControl2* pSessionControl2 = nullptr;
        hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2),
                                            (void**)&pSessionControl2);
        if (SUCCEEDED(hr)) {
            DWORD sessionPID;
            hr = pSessionControl2->GetProcessId(&sessionPID);
            if (SUCCEEDED(hr) && sessionPID != currentPID) {
                ISimpleAudioVolume* pVolume = nullptr;
                hr = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume),
                                                    (void**)&pVolume);
                if (SUCCEEDED(hr)) {
                    float currentVol;
                    hr = pVolume->GetMasterVolume(&currentVol);
                    if (SUCCEEDED(hr)) {
                        float newVol = currentVol * (1.0f - reductionPercent / 100.0f);
                        hr = pVolume->SetMasterVolume(newVol, nullptr);
                        if (SUCCEEDED(hr)) {
                            savedVolumes.push_back({sessionPID, currentVol});
                        }
                    }
                    pVolume->Release();
                }
            }
            pSessionControl2->Release();
        }
        pSessionControl->Release();
    }

cleanup:
    if (pSessionEnumerator) pSessionEnumerator->Release();
    if (pSessionManager) pSessionManager->Release();
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();
    if (coInitialized) CoUninitialize();
}

void RestoreApplicationVolumes() {
    if (savedVolumes.empty()) return;

    HRESULT hr;
    bool coInitialized = false;

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioSessionManager2* pSessionManager = nullptr;
    IAudioSessionEnumerator* pSessionEnumerator = nullptr;
    int sessionCount = 0;

    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        if (hr != RPC_E_CHANGED_MODE) {
            return;
        }
        coInitialized = false;
    } else {
        coInitialized = true;
    }

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) goto cleanup;

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) goto cleanup;

    hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL,
                           nullptr, (void**)&pSessionManager);
    if (FAILED(hr)) goto cleanup;

    hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
    if (FAILED(hr)) goto cleanup;

    hr = pSessionEnumerator->GetCount(&sessionCount);
    if (FAILED(hr)) goto cleanup;

    for (int i = 0; i < sessionCount; ++i) {
        IAudioSessionControl* pSessionControl = nullptr;
        hr = pSessionEnumerator->GetSession(i, &pSessionControl);
        if (FAILED(hr)) continue;

        IAudioSessionControl2* pSessionControl2 = nullptr;
        hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2),
                                            (void**)&pSessionControl2);
        if (SUCCEEDED(hr)) {
            DWORD sessionPID;
            hr = pSessionControl2->GetProcessId(&sessionPID);
            if (SUCCEEDED(hr)) {
                auto it = std::find_if(savedVolumes.begin(), savedVolumes.end(),
                                       [sessionPID](const SessionVolume& sv) {
                                           return sv.pid == sessionPID;
                                       });
                if (it != savedVolumes.end()) {
                    ISimpleAudioVolume* pVolume = nullptr;
                    hr = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume),
                                                        (void**)&pVolume);
                    if (SUCCEEDED(hr)) {
                        hr = pVolume->SetMasterVolume(it->volume, nullptr);
                        pVolume->Release();
                    }
                }
            }
            pSessionControl2->Release();
        }
        pSessionControl->Release();
    }

    savedVolumes.clear();

cleanup:
    if (pSessionEnumerator) pSessionEnumerator->Release();
    if (pSessionManager) pSessionManager->Release();
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();
    if (coInitialized) CoUninitialize();
}