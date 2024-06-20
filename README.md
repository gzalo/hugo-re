# Hugo-RE
Relive the Hugo TV gaming experience from the 90s!

Using an IP phone (or an analog phone with a FXS gateway) you'll be able to call the virtual Hugo hotline and play the games virtually.
Eventually we'll have some AI-generated voices for Hugo and the host (Gaby in Argentina)

**Phase 0**: As seen in Flashparty 2023, with quite a high latency due to the audio processing chain

**Phase 1**: As seen in Cybercirujas federal event 2023, way lower latency using pjsua, quite playable

**Phase 2**: As seen in Cybercirujas event June 2024, introduced a wrapper with full-motion videos and random game selector

![Remake](docs/hugoremake.drawio.png)

- [Game wrapper with full-motion videos](game/)
- [PJSUA (SIP tones to keys)](pjsua-to-keys/)
- [Asterisk config](asterisk-config/)

- [Reverse engineering progress (WIP)](docs/reverse.md)
- [Hugo resource viewer (WIP)](viewer/)
- [DTMF tones to key (outdated)](dtmf-to-keys/)

I'm looking for contributors! If you are interested in:
- AI/LLM
- Reverse engineering old DOS/WIN32 games
- Speech recognition
- 3d modeling and gamedev

...you can join the team and help the project advance!

## Todo: 
- Add subtitles to videos
- After losing game, stop looping again and ask the user to hang up

## Original Hugo Landline phone numbers
- 4317-8501
- 0600-111-0203

### In memoriam - Hugo Avila - 1921-2007

**Disclaimer: Obviously, this project is not official and is unrelated to ITE/Pramer/Promofilm/Magic Kids. Please support the official Hugo franchise by buying their newest games and merch**

Similar projects:
- https://github.com/tuomasth/Hugo_Ski_Game_Finnish_24-2-2023
- https://github.com/supinrus/HugoMega
- https://github.com/supinrus/HugoResourceViewer
- https://github.com/erengaygusuz/hugo-lululu
- https://github.com/ElMiauro/KitoPizzas