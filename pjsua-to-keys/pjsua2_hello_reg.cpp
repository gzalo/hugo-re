#include <pjsua2.hpp>
#include <iostream>
#include <windows.h>

using namespace pj;

int my_snd_dev_id;

void processDtmf(string digit, bool down){
    /*'1': 79,
    '2': 72, # 8
    '3': 81,
    '4': 75,
    '5': 76,
    '6': 77,
    '7': 71,
    '8': 80, # 2
    '9': 73,
    '0': 82,
    '#': # Enter
    '*': # Escape*/
    DWORD flags = down ? 0 : KEYEVENTF_KEYUP;

    if(digit == "1"){
        keybd_event(VK_NUMPAD1, 0, flags, 0);
    } else if (digit == "2"){
        keybd_event(VK_NUMPAD8, 0, flags, 0);
    } else if (digit == "3"){
        keybd_event(VK_NUMPAD3, 0, flags, 0);
    } else if (digit == "4"){
        keybd_event(VK_NUMPAD4, 0, flags, 0);
    } else if (digit == "5"){
        keybd_event(VK_NUMPAD5, 0, flags, 0);
    } else if (digit == "6"){
        keybd_event(VK_NUMPAD6, 0, flags, 0);
    } else if (digit == "7"){
        keybd_event(VK_NUMPAD7, 0, flags, 0);
    } else if (digit == "8"){
        keybd_event(VK_NUMPAD2, 0, flags, 0);
    } else if (digit == "9"){
        keybd_event(VK_NUMPAD9, 0, flags, 0);
    } else if (digit == "0"){
        keybd_event(VK_NUMPAD0, 0, flags, 0);
    } else if (digit == "#"){
        keybd_event(VK_RETURN, 0, flags, 0);
    } else if (digit == "*"){
        keybd_event(VK_ESCAPE, 0, flags, 0);
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

int main(int argc, char **argv)
{
    Endpoint ep;

    ep.libCreate();

    // Initialize endpoint
    EpConfig ep_cfg;
    ep.libInit( ep_cfg );

    // Create SIP transport. Error handling sample is shown
    TransportConfig tcfg;
    tcfg.port = 5060;
    try {
        ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
    } catch (Error &err) {
        std::cout << err.info() << std::endl;
        return 1;
    }

    // Start the library (worker threads etc)
    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <CaptureDevice> <PlaybackDevice> (e.g 4 7)" << std::endl;
        return -1;
    }

    int captureDev = std::stoi(argv[1]);
    int playbackDev = std::stoi(argv[2]);

    ep.audDevManager().setCaptureDev(captureDev);
    ep.audDevManager().setPlaybackDev(playbackDev);

    // Configure an AccountConfig
    AccountConfig acfg;
    acfg.idUri = "sip:105@192.168.0.20";
    acfg.regConfig.registrarUri = "sip:192.168.0.20";
    AuthCredInfo cred("digest", "*", "105", 0, "unsecurepassword");
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
