# Reverse engineering formats

Data was extracted from [HUGO3](https://sites.google.com/view/ajugarconhugopc/juegos/hugo-3?authuser=0)

- Use [QuickBMS](http://aluigi.altervista.org/quickbms.htm) with [Hugo3](http://aluigi.altervista.org/bms/hugo3.bms) script to extract 4HUGO.DAT

# Formats fully decoded

- PCX or PCC: Picture exchange format. Credits.pcx with a sepia effect are used for showing scores in the TV show.
- RAW or PIC or BLK: [HSI Raw](http://justsolve.archiveteam.org/wiki/HSI_Raw) pictures, can be viewed in XNView. "mhwanh" header. vgapal.raw, pcpal.raw are used as palletes for other files.
- SND: raw signed 8 bit PCM, 1 channel, 22050 Hz. Can be opened with audacity. kult.snd is weird

- OFS/OFF: Usually comes with an associated BRS file. It just has `N` 4 byte offsets into the other file, could point to frames or different images

# Formats partially decoded
- BRS: Animation files
    - contains N back-to-back frames, each one starts with a (W H X Y) 16 byte header. Uncompressed palletized byte data

- PBR: [This format contains images, documented here](reverse-pbr.md)

# Unknown formats
- CBR: Likely images?

- TIL/TI2/TI4: tiled animations? first 0x20 header, 0x20-0x320 seems pallete, 0x320 on data. `Intro.til` seems to show typical blue and white ITE logo, but parsing it is harder.

- DAT/BIN: binary data about game logic. Very sparse. Variations for TV and Arcade modes that have different difficulty. Seems to be made of blocks of 12 bytes, no headers
    - poleX.bin has a different structure: only 00s at the start, then 10h-16h and 19h

- BSH: only in pladebag: has some texts: FORM/GRAB/DRNG
