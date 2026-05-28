export class Tween {
  static map(value: number, inMin: number, inMax: number, outMin: number, outMax: number): number {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
  }

  static mapBitcrush(value: number, inMin: number, inMax: number, outMin: number, outMax: number): number {
    const normalized = Tween.map(value, inMin, inMax, 0, 1);
    if (normalized > 0.75) return outMin;
    if (normalized > 0.5)  return outMin * 2;
    if (normalized > 0.25) return outMin * 4;
    return outMax;
  }

  static mapEaseIn(value: number, inMin: number, inMax: number, outMin: number, outMax: number): number {
    let normalized = (value - inMin) / (inMax - inMin);
    if (normalized < 0) normalized = 0;
    if (normalized > 1) normalized = 1;
    const eased = normalized ** 2;
    return eased * (outMax - outMin) + outMin;
  }
}
