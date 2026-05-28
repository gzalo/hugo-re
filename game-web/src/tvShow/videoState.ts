/** Port of game/tv_show/video_state.py */

import { State, type StateClass } from '../state';
import { AudioHelper } from '../audioHelper';
import type { VideoWrapper } from './tvShowResources';
import type { GameData } from '../gameData';
import type { PhoneEvents } from '../phoneEvents';

export class VideoState extends State {
  protected video: VideoWrapper;
  protected loop: boolean;
  protected hasLooped = false;

  private audioResource: string | null;
  private audioInstanceId: number | null = null;

  constructor(
    context: GameData,
    video: VideoWrapper,
    loop = false,
    audioResource: string | null = null,
  ) {
    super(context);
    this.video = video;
    this.loop = loop;
    this.audioResource = audioResource;
  }

  onEnter(): void {
    super.onEnter();
    this.video.restart();
    if (this.audioResource) {
      this.audioInstanceId = AudioHelper.play(this.audioResource);
    }
  }

  processEvents(_events: PhoneEvents): StateClass | null {
    if (this.loop && !this.video.active) {
      this.video.restart();
      this.hasLooped = true;
      if (this.audioResource && this.audioInstanceId === null) {
        this.audioInstanceId = AudioHelper.play(this.audioResource);
      }
    }
    return null;
  }

  render(ctx: OffscreenCanvasRenderingContext2D): void {
    // readyState < HAVE_CURRENT_DATA (2) means no frame is available yet
    if (this.video.element.readyState >= 2) {
      this.video.draw(ctx, 0, 0);
    }
  }

  onExit(): void {
    super.onExit();
    this.video.stop();
    if (this.audioInstanceId !== null) {
      AudioHelper.stop(this.audioInstanceId);
      this.audioInstanceId = null;
    }
  }

  protected videoEnded(): boolean {
    return !this.video.active;
  }
}
