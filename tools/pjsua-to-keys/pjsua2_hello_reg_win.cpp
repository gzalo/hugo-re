#include <pjsua2.hpp>
#include <iostream>
#include <windows.h>

using namespace pj;

int phoneId;

int offHookId[] = {0x3b,0x3d,0x3f,0x41};
int hungUpId[] = {0x3c,0x3e,0x40,0x42};
int key0[] = {0x2c, 0x2d, 0x2e, 0x52};
int key1[] = {0x02, 0x05, 0x08, 0x4f};
int key2[] = {0x03, 0x06, 0x09, 0x50};
int key3[] = {0x04,0x07,0x0a,0x51};
int key4[] = {0x10,0x13,0x16,0x4b};
int key5[] = {0x11,0x14,0x17,0x4c};
int key6[] = {0x12,0x15,0x18,0x4d};
int key7[] = {0x1E,0x21,0x24,0x47};
int key8[] = {0x1F,0x22,0x25,0x48};
int key9[] = {0x20,0x23,0x26,0x49};

INPUT createScanCodeEvent(WORD scancode, bool isDown){
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = scancode;
    input.ki.dwFlags = (isDown ? 0 : KEYEVENTF_KEYUP) | KEYEVENTF_SCANCODE;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    return input;
}

void processDtmf(string digit, bool down){
    INPUT input;

    if(digit == "1"){
        input = createScanCodeEvent(key1[phoneId], down);
    } else if (digit == "2"){
        input = createScanCodeEvent(key2[phoneId], down);
    } else if (digit == "3"){
        input = createScanCodeEvent(key3[phoneId], down);
    } else if (digit == "4"){
        input = createScanCodeEvent(key4[phoneId], down);
    } else if (digit == "5"){
        input = createScanCodeEvent(key5[phoneId], down);
    } else if (digit == "6"){
        input = createScanCodeEvent(key6[phoneId], down);
    } else if (digit == "7"){
        input = createScanCodeEvent(key7[phoneId], down);
    } else if (digit == "8"){
        input = createScanCodeEvent(key8[phoneId], down);
    } else if (digit == "9"){
        input = createScanCodeEvent(key9[phoneId], down);
    } else if (digit == "0"){
        input = createScanCodeEvent(key0[phoneId], down);
    }

    SendInput(1, &input, sizeof(INPUT));
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
            {
                INPUT input = createScanCodeEvent(offHookId[phoneId], true);
                SendInput(1, &input, sizeof(INPUT));
            }
            Sleep(50);
            {
                INPUT input = createScanCodeEvent(offHookId[phoneId], false);
                SendInput(1, &input, sizeof(INPUT));
            }
        }
        if (ci.state == PJSIP_INV_STATE_DISCONNECTED)
        {
            std::cout << "Call is disconnected." << std::endl;
            {
                INPUT input = createScanCodeEvent(hungUpId[phoneId], true);    
                SendInput(1, &input, sizeof(INPUT));
            }
            Sleep(50);
            {
                INPUT input = createScanCodeEvent(hungUpId[phoneId], false);     
                SendInput(1, &input, sizeof(INPUT));       
            }
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
        aud_med.startTransmit(play_dev_med);
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
    tcfg.port = 5060 + phoneId;
    try {
        ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
    } catch (Error &err) {
        std::cout << err.info() << std::endl;
        return 1;
    }

    // Start the library (worker threads etc)
    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;

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

    // This will implicitly shutdown the library
    return 0;
}
