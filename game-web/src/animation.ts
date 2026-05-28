/** Port of game/animation.py */

export class Animation {
  static getSyncFrame(animations: ImageBitmap[], syncs: number[], frameIndex: number): ImageBitmap {
    if (frameIndex >= syncs.length) frameIndex = syncs.length - 1;
    let frame = syncs[frameIndex] - 1;
    if (frame < 0) { console.warn('Animation index less than 0!'); frame = 0; }
    if (frame >= animations.length) { console.warn('Animation index out of range!'); frame = animations.length - 1; }
    return animations[frame];
  }

  static getFrame(animations: ImageBitmap[], frameIndex: number): ImageBitmap {
    if (frameIndex >= animations.length) {
      console.warn('Animation index out of range!');
      frameIndex = animations.length - 1;
    }
    return animations[frameIndex];
  }
}
