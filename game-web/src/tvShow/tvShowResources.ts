/** Port of game/tv_show/tv_show_resources.py */

import { loadSurfaceRes } from '../resource';
import { Config } from '../config';

export class VideoWrapper {
  readonly element: HTMLVideoElement;

  constructor(src: string) {
    this.element = document.createElement('video');
    this.element.src = src;
    this.element.muted = true;   // audio handled separately via Web Audio
    this.element.preload = 'auto';
  }

  /** True while the video is playing (not paused / ended). */
  get active(): boolean {
    return !this.element.paused && !this.element.ended;
  }

  /** Restart from the beginning and start playing. */
  restart(): void {
    this.element.currentTime = 0;
    void this.element.play();
  }

  /** Stop playback and rewind. */
  stop(): void {
    this.element.pause();
    this.element.currentTime = 0;
  }

  /** Draw the current video frame to a 2D context at (x, y). */
  draw(ctx: OffscreenCanvasRenderingContext2D, x: number, y: number): void {
    ctx.drawImage(this.element, x, y);
  }
}

export class TvShowResources {
  static press5: ImageBitmap | null = null;
  static instructions: Record<string, ImageBitmap> = {};

  static videosAttract:     Record<string, VideoWrapper> = {};
  static videosInitial:     Record<string, VideoWrapper> = {};
  static videosPress5:      Record<string, VideoWrapper> = {};
  static videosGoingScylla: Record<string, VideoWrapper> = {};
  static videosEnding:      Record<string, VideoWrapper> = {};
  static videosHaveLuck:    Record<string, VideoWrapper> = {};

  static audioAttract:     Record<string, string> = {};
  static audioInitial:     Record<string, string> = {};
  static audioPress5:      Record<string, string> = {};
  static audioGoingScylla: Record<string, string> = {};
  static audioEnding:      Record<string, string> = {};
  static audioHaveLuck:    Record<string, string> = {};

  static async init(): Promise<void> {
    TvShowResources.press5 = await loadSurfaceRes('images/press_5.png');

    const instructionPromises = Object.keys(Config.GAMES).map(async (gameName) => {
      const bitmap = await loadSurfaceRes(`images/instruction_${gameName}.png`);
      TvShowResources.instructions[gameName] = bitmap;
    });
    await Promise.all(instructionPromises);

    for (const country of Config.COUNTRIES) {
      const videoPrefix = `${import.meta.env.BASE_URL}videos/${country}/`;
      const audioPrefix = `audio_for_videos/${country}/`;

      TvShowResources.videosAttract[country]     = new VideoWrapper(videoPrefix + 'attract_demo.webm');
      TvShowResources.videosInitial[country]     = new VideoWrapper(videoPrefix + 'hello_hello.webm');
      TvShowResources.videosPress5[country]      = new VideoWrapper(videoPrefix + 'press_5.webm');
      TvShowResources.videosGoingScylla[country] = new VideoWrapper(videoPrefix + 'scylla_cave.webm');
      TvShowResources.videosEnding[country]      = new VideoWrapper(videoPrefix + 'you_lost.webm');
      TvShowResources.videosHaveLuck[country]    = new VideoWrapper(videoPrefix + 'have_luck.webm');

      TvShowResources.audioAttract[country]     = audioPrefix + 'attract_demo.wav';
      TvShowResources.audioInitial[country]     = audioPrefix + 'hello_hello.wav';
      TvShowResources.audioPress5[country]      = audioPrefix + 'press_5.wav';
      TvShowResources.audioGoingScylla[country] = audioPrefix + 'scylla_cave.wav';
      TvShowResources.audioEnding[country]      = audioPrefix + 'you_lost.wav';
      TvShowResources.audioHaveLuck[country]    = audioPrefix + 'have_luck.wav';
    }
  }
}
