from resource import Resource


class CaveResources:
    caselive_til = "CASELIVE.TIL"
    casedie_til = "CASEDIE.TIL"

    her_er_vi = None
    talks = None
    climbs = None
    first_rope = None
    second_rope = None
    third_rope = None
    scylla_leaves = None
    scylla_bird = None
    scylla_ropes = None
    scylla_spring = None
    family_cage = None
    hugo_puff_first = None
    hugo_puff_second = None
    hugo_puff_third = None
    hugo_spring = None
    happy = None
    score_font = None
    hugo = None

    sync_hugo_start = None
    sync_hugo_die = None

    trappe_snak = None
    nu_kommer_jeg = None
    afskylia_snak = None
    hugo_katapult = None
    hugo_skyd_ud = None
    afskylia_skyd_ud = None
    hugoline_tak = None

    stemning = None
    fodtrin1 = None
    fodtrin2 = None
    hiv_i_reb = None
    fjeder = None
    pre_puf = None
    puf = None
    tast_trykket = None
    pre_fanfare = None
    fanfare = None
    fugle_skrig = None
    trappe_grin = None
    skrig = None
    score_counter = None
    bg_music = None


    @staticmethod
    def init():
        CaveResources.her_er_vi = Resource.load_speak("RopeOutroData", "002-05.wav")  # Help us Hugo
        CaveResources.talks = Resource.load_surfaces("RopeOutroData", "STAIRS.TIL", 0, 12)
        CaveResources.climbs = Resource.load_surfaces("RopeOutroData", "STAIRS.TIL", 11, 51)
        CaveResources.first_rope = Resource.load_surfaces("RopeOutroData", CaveResources.caselive_til, 0, 32)
        CaveResources.second_rope = Resource.load_surfaces("RopeOutroData", CaveResources.caselive_til, 33, 72)
        CaveResources.third_rope = Resource.load_surfaces("RopeOutroData", CaveResources.caselive_til, 73, 121)
        CaveResources.scylla_leaves = Resource.load_surfaces("RopeOutroData", CaveResources.caselive_til, 122, 177)
        CaveResources.scylla_bird = Resource.load_surfaces("RopeOutroData", CaveResources.caselive_til, 178, 240)
        CaveResources.scylla_ropes = Resource.load_surfaces("RopeOutroData", CaveResources.caselive_til, 241, 283)
        CaveResources.scylla_spring = Resource.load_surfaces("RopeOutroData", CaveResources.caselive_til, 284, 318)
        CaveResources.family_cage = Resource.load_surfaces("RopeOutroData", CaveResources.caselive_til, 319, 352)
        CaveResources.hugo_puff_first = Resource.load_surfaces("RopeOutroData", CaveResources.casedie_til, 122, 166)
        CaveResources.hugo_puff_second = Resource.load_surfaces("RopeOutroData", CaveResources.casedie_til, 167, 211)
        CaveResources.hugo_puff_third = Resource.load_surfaces("RopeOutroData", CaveResources.casedie_til, 212, 256)
        CaveResources.hugo_spring = Resource.load_surfaces("RopeOutroData", CaveResources.casedie_til, 257, 295)
        CaveResources.happy = Resource.load_surfaces("RopeOutroData", "HAPPY.TIL", 0, 111)
        CaveResources.score_font = Resource.load_surfaces("RopeOutroData", "SCORE.cgf", 0, 9)
        CaveResources.hugo = Resource.load_surfaces("RopeOutroData", "hugo.cgf", 0, 0)

        CaveResources.sync_hugo_start = Resource.load_sync("RopeOutroData", "002-06.oos")
        CaveResources.sync_hugo_die = Resource.load_sync("RopeOutroData", "002-09.oos")

        CaveResources.trappe_snak = Resource.load_speak("RopeOutroData", "002-06.wav")  # I feel like this is dangerous
        CaveResources.nu_kommer_jeg = Resource.load_speak("RopeOutroData", "002-07.wav")  # Hello is there anyone here
        CaveResources.afskylia_snak = Resource.load_speak("RopeOutroData", "002-08.wav")  # Go ahead and choose
        CaveResources.hugo_katapult = Resource.load_speak("RopeOutroData", "002-09.wav")  # I'll be back
        CaveResources.hugo_skyd_ud = Resource.load_speak("RopeOutroData", "002-10.wav")  # Hugo screams on lost
        CaveResources.afskylia_skyd_ud = Resource.load_speak("RopeOutroData", "002-11.wav")  # Scylla screams on win
        CaveResources.hugoline_tak = Resource.load_speak("RopeOutroData", "002-12.wav")  # We are free

        CaveResources.stemning = Resource.load_sfx("RopeOutroData", "BA-13.WAV")  # Background chimes
        CaveResources.fodtrin1 = Resource.load_sfx("RopeOutroData", "BA-15.WAV")  # Step 1
        CaveResources.fodtrin2 = Resource.load_sfx("RopeOutroData", "BA-16.WAV")  # Step 2
        CaveResources.hiv_i_reb = Resource.load_sfx("RopeOutroData", "BA-17.WAV")  # Rope used
        CaveResources.fjeder = Resource.load_sfx("RopeOutroData", "BA-18.WAV")  # Spring
        CaveResources.pre_puf = Resource.load_sfx("RopeOutroData", "BA-21.WAV")  # Anticipation
        CaveResources.puf = Resource.load_sfx("RopeOutroData", "BA-22.WAV")  # Smoke puff
        CaveResources.tast_trykket = Resource.load_sfx("RopeOutroData", "BA-24.WAV")  # Waiting for you ding
        CaveResources.pre_fanfare = Resource.load_sfx("RopeOutroData", "BA-101.WAV")  # Choose something background
        CaveResources.fanfare = Resource.load_sfx("RopeOutroData", "BA-102.WAV")  # Correct ending (This was starting to ...)
        CaveResources.fugle_skrig = Resource.load_sfx("RopeOutroData", "BA-104.WAV")  # Scylla turned into bird
        CaveResources.trappe_grin = Resource.load_sfx("RopeOutroData", "HEXHAHA.WAV")  # Odd laugh
        CaveResources.skrig = Resource.load_sfx("RopeOutroData", "SKRIG.WAV")  # Scilla screams
        CaveResources.score_counter = Resource.load_sfx("RopeOutroData", "COUNTER.WAV")  # Score counter increase
        CaveResources.bg_music = Resource.load_sfx("RopeOutroData",  "a_party_for_me.mp3")  # BG music, downloaded from https://www.youtube.com/watch?v=maJVRkefBhw
