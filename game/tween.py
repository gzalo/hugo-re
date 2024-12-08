class Tween:
    @staticmethod
    def map(value, in_min, in_max, out_min, out_max):
        return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

    @staticmethod
    def map_bitcrush(self, value, in_min, in_max, out_min, out_max):
        value = self.map(value, in_min, in_max, 0, 1)
        if value > 0.75:
            return out_min
        elif value > 0.5:
            return out_min * 2
        elif value > 0.25:
            return out_min * 4
        return out_max

    @staticmethod
    def map_ease_in(value, in_min, in_max, out_min, out_max):
        normalized = (value - in_min) / (in_max - in_min)
        if normalized < 0: normalized = 0
        if normalized > 1: normalized = 1
        eased = normalized ** 2
        return eased * (out_max - out_min) + out_min