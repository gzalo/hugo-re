#define PJ_IS_LITTLE_ENDIAN 1
#define PJ_IS_BIG_ENDIAN 0
#include <pjsua2.hpp>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <libevdev-1.0/libevdev/libevdev-uinput.h>

using namespace pj;

int phoneId;
struct libevdev_uinput *uidev;

int offHookId[] = {KEY_F1,KEY_F3,KEY_F5,KEY_F7};
int hungUpId[] = {KEY_F2,KEY_F4,KEY_F6,KEY_F8};
int key0[] = {KEY_Z, KEY_X, KEY_C, KEY_KP0};
int key1[] = {KEY_1, KEY_4, KEY_7, KEY_KP1};
int key2[] = {KEY_2, KEY_5, KEY_8, KEY_KP2};
int key3[] = {KEY_3,KEY_6,KEY_9,KEY_KP3};
int key4[] = {KEY_Q,KEY_R,KEY_U,KEY_KP4};
int key5[] = {KEY_W,KEY_T,KEY_I,KEY_KP5};
int key6[] = {KEY_E,KEY_Y,KEY_O,KEY_KP6};
int key7[] = {KEY_A,KEY_F,KEY_J,KEY_KP7};
int key8[] = {KEY_S,KEY_G,KEY_K,KEY_KP8};
int key9[] = {KEY_D,KEY_H,KEY_L,KEY_KP9};

void writeScanCodeEvent(int keycode, bool isDown){
    libevdev_uinput_write_event(uidev, EV_KEY, keycode, isDown ? 1 : 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0); // Synchronize
}

void processDtmf(string digit, bool down){
    if(digit == "1"){
        writeScanCodeEvent(key1[phoneId], down);
    } else if (digit == "2"){
        writeScanCodeEvent(key2[phoneId], down);
    } else if (digit == "3"){
        writeScanCodeEvent(key3[phoneId], down);
    } else if (digit == "4"){
        writeScanCodeEvent(key4[phoneId], down);
    } else if (digit == "5"){
        writeScanCodeEvent(key5[phoneId], down);
    } else if (digit == "6"){
        writeScanCodeEvent(key6[phoneId], down);
    } else if (digit == "7"){
        writeScanCodeEvent(key7[phoneId], down);
    } else if (digit == "8"){
        writeScanCodeEvent(key8[phoneId], down);
    } else if (digit == "9"){
        writeScanCodeEvent(key9[phoneId], down);
    } else if (digit == "0"){
        writeScanCodeEvent(key0[phoneId], down);
    }
}

class MyAudioMediaPort: public AudioMediaPort
{
    virtual void onFrameRequested(MediaFrame &frame)
    {
        // Give the input frame here
        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        // frame.buf.assign(frame.size, 'c');
    }

    virtual void onFrameReceived(MediaFrame &frame)
    {
        PJ_UNUSED_ARG(frame);
        // Process the incoming frame here
    }
};

class MyCall : public Call
{

public:
    MyCall(Account &acc, int call_id = PJSUA_INVALID_ID)
    : Call(acc, call_id)
    { 
    }  

    void onCallState(pj::OnCallStateParam &prm) override
    {
        CallInfo ci = getInfo();
        std::cout << "Call state: " << ci.stateText << std::endl;

        if (ci.state == PJSIP_INV_STATE_CONFIRMED)
        {
            std::cout << "Call is confirmed. You can start receiving DTMF codes." << std::endl;
            writeScanCodeEvent(offHookId[phoneId], true);
            usleep(50000);
            writeScanCodeEvent(offHookId[phoneId], false);
        }
        if (ci.state == PJSIP_INV_STATE_DISCONNECTED)
        {
            std::cout << "Call is disconnected." << std::endl;
            writeScanCodeEvent(hungUpId[phoneId], true);
            usleep(50000);
            writeScanCodeEvent(hungUpId[phoneId], false);
        }
    }

    void onDtmfEvent(pj::OnDtmfEventParam &prm) override
    {
        if(!(prm.flags & PJMEDIA_STREAM_DTMF_IS_UPDATE)) { // keydown
            std::cout << "DTMF digit keydown: " << prm.digit << std::endl;
            processDtmf(prm.digit, true);
        } 
        if(prm.flags & PJMEDIA_STREAM_DTMF_IS_END) { // keyup
            std::cout << "DTMF digit keyup: " << prm.digit << std::endl;
            processDtmf(prm.digit, false);
        }
    }

    void onCallMediaState(OnCallMediaStateParam& prm)
    {
        AudioMedia& play_dev_med = Endpoint::instance().audDevManager().getPlaybackDevMedia();
        AudioMedia& record_dev_med = Endpoint::instance().audDevManager().getCaptureDevMedia();
        AudioMedia aud_med;

        try {
            // Get the first audio media
            aud_med = getAudioMedia(-1);
        } catch(...) {
            std::cout << "Failed to get audio media" << std::endl;
            return;
        }

        // And this will connect the call audio media to the sound device/speaker
        // aud_med.startTransmit(play_dev_med);
        record_dev_med.startTransmit(aud_med);
    }
};

class MyAccount : public Account {
public:
    virtual void onRegState(OnRegStateParam &prm) {
        AccountInfo ai = getInfo();
        std::cout << (ai.regIsActive? "*** Register:" : "*** Unregister:")
                  << " code=" << prm.code << std::endl;
    }

    virtual void onIncomingCall(OnIncomingCallParam &iprm) {
        Call *call = new MyCall(*this, iprm.callId);
        CallOpParam prm;
        prm.statusCode = PJSIP_SC_OK;
        call->answer(prm);
    }

};

string username[4] = {
    "110",
    "111",
    "112",
    "113"
};

int main(int argc, char **argv)
{

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << "<PhoneId> <CaptureDevice> <PlaybackDevice> (e.g 0 4 7)" << std::endl;
        return -1;
    }

    phoneId = std::stoi(argv[1]);
    int captureDev = std::stoi(argv[2]);
    int playbackDev = std::stoi(argv[3]);

    Endpoint ep;

    ep.libCreate();

    // Initialize endpoint
    EpConfig ep_cfg;
    ep.libInit( ep_cfg );

    // Create SIP transport. Error handling sample is shown
    TransportConfig tcfg;
    tcfg.port = 5070 + phoneId;
    try {
        ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
    } catch (Error &err) {
        std::cout << err.info() << std::endl;
        return 1;
    }

    struct libevdev *dev = libevdev_new();
    int err;

    if (!dev) {
        fprintf(stderr, "Failed to initialize libevdev\n");
        return 1;
    }

    // Set up the device
    libevdev_set_name(dev, "Virtual Keyboard");
    libevdev_enable_event_type(dev, EV_KEY);

    libevdev_enable_event_code(dev, EV_KEY, KEY_F1, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_F3, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_F5, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_F7, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_F2, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_F4, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_F6, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_F8, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_Z, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_X, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_C, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_KP0, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_1, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_4, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_7, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_KP1, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_2, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_5, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_8, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_KP2, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_3, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_6, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_9, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_KP3, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_Q, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_R, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_U, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_KP4, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_W, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_T, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_I, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_KP5, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_E, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_Y, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_O, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_KP6, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_A, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_F, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_J, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_KP7, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_S, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_G, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_K, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_KP8, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_D, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_H, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_L, NULL);
    libevdev_enable_event_code(dev, EV_KEY, KEY_KP9, NULL);

    // Create the virtual device
    err = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
    libevdev_free(dev);
    if (err != 0) {
        fprintf(stderr, "Failed to create uinput device: %s\n", strerror(-err));
        return 1;
    }

    // Start the library (worker threads etc)
    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;

try {
    // Get the number of audio devices
    unsigned int devCount = ep.audDevManager().getDevCount();
    std::cout << "Total Audio Devices: " << devCount << std::endl;

    std::cout << "=== INPUT DEVICES ===" << std::endl;

    // Enumerate and print input devices
    for (unsigned int i = 0; i < devCount; ++i) {
        pj::AudioDevInfo devInfo = ep.audDevManager().getDevInfo(i);
        
        // Check if it's an input device
        if (devInfo.inputCount > 0) {
            std::cout << "Device " << i << ":" << std::endl;
            std::cout << "  Name: " << devInfo.name << std::endl;
            std::cout << "  Input Channels: " << devInfo.inputCount << std::endl;
            std::cout << std::endl;
        }
    }

    std::cout << "=== OUTPUT DEVICES ===" << std::endl;

    // Enumerate and print output devices
    for (unsigned int i = 0; i < devCount; ++i) {
        pj::AudioDevInfo devInfo = ep.audDevManager().getDevInfo(i);
        
        // Check if it's an output device
        if (devInfo.outputCount > 0) {
            std::cout << "Device " << i << ":" << std::endl;
            std::cout << "  Name: " << devInfo.name << std::endl;
            std::cout << "  Output Channels: " << devInfo.outputCount << std::endl;
            std::cout << std::endl;
        }
    }

    // Set capture and playback devices (as in your original code)
    ep.audDevManager().setCaptureDev(captureDev);
    ep.audDevManager().setPlaybackDev(playbackDev);

} catch (pj::Error& err) {
    std::cerr << "Error enumerating audio devices: " << err.info() << std::endl;
}

    ep.audDevManager().setCaptureDev(captureDev);
    ep.audDevManager().setPlaybackDev(playbackDev);

    // Configure an AccountConfig
    AccountConfig acfg;
    acfg.idUri = "sip:" + username[phoneId] + "@10.0.0.1";
    acfg.regConfig.registrarUri = "sip:10.0.0.1";
    AuthCredInfo cred("digest", "*", username[phoneId], 0, "unsecurepassword");
    acfg.sipConfig.authCreds.push_back( cred );

    // Create the account
    MyAccount *acc = new MyAccount;
    acc->create(acfg);

    bool quit = false;
    while(!quit){
        // Here we don't have anything else to do..
        pj_thread_sleep(10000);
    }

    // Delete the account. This will unregister from server
    delete acc;

    libevdev_uinput_destroy(uidev);

    // This will implicitly shutdown the library
    return 0;
}
