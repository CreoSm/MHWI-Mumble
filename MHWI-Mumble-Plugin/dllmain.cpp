#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <sstream>
#include<string>
#include <random>


using namespace std;


std::string generate_random_string(size_t length) {
    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);

    std::string random_string;
    for (size_t i = 0; i < length; ++i) {
        random_string += characters[distribution(generator)];
    }

    return random_string;
}

// START MUMBLE CRAP //
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <fcntl.h> /* For O_* constants */
#endif // _WIN32

struct LinkedMem {
#ifdef _WIN32
    UINT32	uiVersion;
    DWORD	uiTick;
#else
    uint32_t uiVersion;
    uint32_t uiTick;
#endif
    float	fAvatarPosition[3];
    float	fAvatarFront[3];
    float	fAvatarTop[3];
    wchar_t	name[256];
    float	fCameraPosition[3];
    float	fCameraFront[3];
    float	fCameraTop[3];
    wchar_t	identity[256];
#ifdef _WIN32
    UINT32	context_len;
#else
    uint32_t context_len;
#endif
    unsigned char context[256];
    wchar_t description[2048];
};

LinkedMem* lm = NULL;

void initMumble() {
    printf("Mumble Link Initiated. . .\n");
#ifdef _WIN32
    HANDLE hMapObject = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"MumbleLink");
    if (hMapObject == NULL)
        return;

    lm = (LinkedMem*)MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));
    if (lm == NULL) {
        CloseHandle(hMapObject);
        hMapObject = NULL;
        return;
    }
#else
    char memname[256];
    snprintf(memname, 256, "/MumbleLink.%d", getuid());

    int shmfd = shm_open(memname, O_RDWR, S_IRUSR | S_IWUSR);

    if (shmfd < 0) {
        return;
    }

    lm = (LinkedMem*)(mmap(NULL, sizeof(struct LinkedMem), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0));

    if (lm == (void*)(-1)) {
        lm = NULL;
        return;
    }
#endif
}

float AvatarXPos;
float AvatarYPos;
float AvatarZPos;

float AvatarFrontX;
float AvatarFrontY;
float AvatarFrontZ;

float CameraXPos;
float CameraYPos;
float CameraZPos;

float CameraFrontX;
float CameraFrontY;
float CameraFrontZ;

const wchar_t* UniqueID;

void updateMumble() {
    if (!lm)
        return;

    if (lm->uiVersion != 2) {
        wcsncpy_s(lm->name, L"MHWILDS Mumble Linker", 256);
        wcsncpy_s(lm->description, L"MHWilds positional voice chat", 2048);
        lm->uiVersion = 2;
    }
    lm->uiTick++;

    // Left handed coordinate system.
    // X positive towards "right".
    // Y positive towards "up".
    // Z positive towards "front".
    //
    // 1 unit = 1 meter

    // Unit vector pointing out of the avatar's eyes aka "At"-vector.
    lm->fAvatarFront[0] = -AvatarFrontX;
    lm->fAvatarFront[1] = -AvatarFrontY;
    lm->fAvatarFront[2] = -AvatarFrontZ;

    // Unit vector pointing out of the top of the avatar's head aka "Up"-vector (here Top points straight up).
    lm->fAvatarTop[0] = 0.0f;
    lm->fAvatarTop[1] = 1.0f;
    lm->fAvatarTop[2] = 0.0f;

    // Position of the avatar
    lm->fAvatarPosition[0] = AvatarXPos;
    lm->fAvatarPosition[1] = AvatarYPos+2;
    lm->fAvatarPosition[2] = AvatarZPos;

    // Same as avatar
    lm->fCameraPosition[0] = CameraXPos;
    lm->fCameraPosition[1] = CameraYPos;
    lm->fCameraPosition[2] = CameraZPos;

    lm->fCameraFront[0] = -CameraFrontX;
    lm->fCameraFront[1] = -CameraFrontY;
    lm->fCameraFront[2] = -CameraFrontZ;

    lm->fCameraTop[0] = 0.0f;
    lm->fCameraTop[1] = 1.0f;
    lm->fCameraTop[2] = 0.0f;

    // Identifier which uniquely identifies a certain player in a context (e.g. the ingame name).
    wcsncpy_s(lm->identity, UniqueID, 256);
    // Context should be equal for players which should be able to hear each other positional and
    // differ for those who shouldn't (e.g. it could contain the server+port and team)
    memcpy(lm->context, "Monster Hunter Wilds Mumble Linked", 16);
    lm->context_len = 16;
}

// END MUMBLE CRAP //


bool ShouldWarnOfMasterCamera = true;
bool ShouldWarnOfMasterPlayer = false;
using namespace reframework;


void UpdateOurClasses() { // This function FUCKING SUCKS!! //
    // For if players are on title screen or something
    AvatarXPos = 0;
    AvatarYPos = 99999;
    AvatarZPos = 0;

    CameraXPos = 0;
    CameraYPos = 99999;
    CameraZPos = -1;

    AvatarFrontX = 0;
    AvatarFrontY = 0;
    AvatarFrontZ = -1;

    CameraFrontX = 0;
    CameraFrontY = 0;
    CameraFrontZ = 1;


    auto& api = API::get();
    auto tdb = api->tdb();
    auto vm_context = api->get_vm_context();

    auto CameraManager = api->get_managed_singleton("app.CameraManager");
    auto CameraManager_type = tdb->find_type("app.CameraManager");

    auto _MasterPlCameraField = CameraManager_type->find_field("_MasterPlCamera");
    auto _MasterPlCamera = _MasterPlCameraField->get_data<API::ManagedObject*>(CameraManager);






    auto GameObjectType = tdb->find_type("via.GameObject");
    auto GetTransform = GameObjectType->find_method("get_Transform");

    auto TransformType = tdb->find_type("via.Transform");
    auto GetPosition = TransformType->find_method("get_Position");

    auto PositionType = tdb->find_type("via.vec3");

    auto xField = PositionType->find_field("x");
    auto yField = PositionType->find_field("y");
    auto zField = PositionType->find_field("z");

    if (_MasterPlCamera == nullptr) {
        if (ShouldWarnOfMasterCamera) {
            printf("Waiting for MasterPlayerCamera\n");
            ShouldWarnOfMasterCamera = false;
        }
    } else {
        if (!ShouldWarnOfMasterCamera) {
            printf("Got MasterPlayerCamera\n");
            ShouldWarnOfMasterCamera = true;
        }

        auto _MasterPlCamera_Type = tdb->find_type("app.PlayerCameraController");
        auto _OwnerField = _MasterPlCamera_Type->find_field("_Owner");
        auto MasterPlayer = _OwnerField->get_data<API::ManagedObject*>(_MasterPlCamera);

        auto get_GameObject = _MasterPlCamera_Type->find_method("get_GameObject");
        auto PlayerCameraController = get_GameObject->call<API::ManagedObject*>(vm_context, _MasterPlCamera);


        auto get_Forward = _MasterPlCamera_Type->find_method("get_Forward");
        


        if (MasterPlayer == nullptr or PlayerCameraController == nullptr) {
            if (ShouldWarnOfMasterPlayer) {
                printf("Master Player or PlayerCameraController Not Found\n");
                ShouldWarnOfMasterPlayer = false;
            }
        
        } else {
            if (!ShouldWarnOfMasterPlayer) {
                printf("Got Master Player PlayerCameraController. . .\n");
                ShouldWarnOfMasterPlayer = true;
            }


            auto MasterPlayerTransform = GetTransform->call<API::ManagedObject*>(vm_context, MasterPlayer);
            auto PlayerCameraTransform = GetTransform->call<API::ManagedObject*>(vm_context, PlayerCameraController);

            if (MasterPlayerTransform == nullptr) {

            }
            else {
                
                
                auto PlayerPosition = GetPosition->call<API::ManagedObject*>(vm_context, MasterPlayerTransform);
                


                if (PlayerPosition == nullptr) {
                }
                else { // Fucking finally get the damn positions
                    // Player Position //
                    AvatarXPos = xField->get_data<float>(PlayerPosition, true); // CHARACTER X
                    AvatarYPos = yField->get_data<float>(PlayerPosition, true); // CHARACTER Y
                    AvatarZPos = zField->get_data<float>(PlayerPosition, true); // CHARACTER Z
                    
                    // Camera Position //
                    auto CameraPosition = GetPosition->call<API::ManagedObject*>(vm_context, PlayerCameraTransform);
                    CameraXPos = xField->get_data<float>(CameraPosition, true); // CAMERA X
                    CameraYPos = yField->get_data<float>(CameraPosition, true); // CAMERA Y
                    CameraZPos = zField->get_data<float>(CameraPosition, true); // CAMERA Z

                    // Camera Forward //
                    auto CameraForwardPosition = get_Forward->call<API::ManagedObject*>(vm_context, _MasterPlCamera);
                    CameraFrontX = xField->get_data<float>(CameraForwardPosition, true); // CAMERA FORWARD X
                    CameraFrontY = yField->get_data<float>(CameraForwardPosition, true); // CAMERA FORWARD Y
                    CameraFrontZ = zField->get_data<float>(CameraForwardPosition, true); // CAMERA FORWARD Z

                    AvatarFrontX = CameraFrontX;
                    AvatarFrontY = CameraFrontY;
                    AvatarFrontZ = CameraFrontZ;
                }
            }
        }

    }

    updateMumble();
}

extern "C" __declspec(dllexport) bool reframework_plugin_initialize(const REFrameworkPluginInitializeParam* param) {
    string randuid = generate_random_string(16);
    wstring wide_string = wstring(randuid.begin(), randuid.end());
    UniqueID = wide_string.c_str();

    API::initialize(param);
    const auto functions = param->functions;
    auto& api = API::get();
    const auto tdb = api->tdb();

    auto vm_context = api->get_vm_context();

    //AllocConsole(); // Used for debugging stuff initially.
    FILE* pCout;
    freopen_s(&pCout, "conout$", "w", stdout);
    printf("MHWI Spatial Mumble Plugin Loaded!!!\n");

    functions->on_pre_application_entry("UpdateBehavior", UpdateOurClasses);

 
    initMumble();

    //auto MasterPlayer = PlayerManager->getMasterPlayer();

    return true;
}