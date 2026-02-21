import global_state
from audio_helper import AudioHelper
from game_data import GameData
from phone_events import PhoneEvents
from scoreboard.scoreboard_resources import ScoreboardResources


class ScoreboardGame:
    """
    Scoreboard stage that displays score breakdown between forest and cave.

    Shows 5 rows sequentially from top to bottom:
    1. Sack score (normal sacks * 100)
    2. End bonus (if reached end: 15000)
    3. Golden sack bonus (golden sacks * 250)
    4. Lives lost penalty (lives lost * -100, with obstacle icons)
    5. Total

    Hugo animation moves from top to bottom as rows are revealed.
    """

    # Timing constants
    ROW_DELAY = 1.5  # seconds between starting each row
    ROLL_SPEED = 50  # points per frame to roll
    HUGO_ANIM_FPS = 20  # Hugo animation frame rate

    # Score values
    SACK_SCORE = 100
    GOLDEN_SACK_SCORE = 250
    END_BONUS = 1500
    LIFE_LOST_PENALTY = 100

    # Layout constants
    SCORE_X = 192  # X position for score numbers (right-aligned area)
    SCORE_DIGIT_SPACING = 16
    ICON_X = 100  # X position for row icons (right of Hugo)
    ROW_HEIGHT = 49  # Vertical spacing between rows
    FIRST_ROW_Y = 3  # Y position of first row

    # Hugo position
    HUGO_X = 0
    HUGO_START_Y = 0
    HUGO_END_Y = 180

    def __init__(self, context: GameData):
        self.context = context
        self.start_time = global_state.frame_time
        self.ended = False

        # Calculate actual scores
        self.sack_score = context.forest_normal_sacks_collected * self.SACK_SCORE
        self.end_bonus = self.END_BONUS if context.forest_reached_end else 0
        self.golden_score = context.forest_golden_sacks_collected * self.GOLDEN_SACK_SCORE

        lives_lost = 3 - context.forest_lives
        self.lives_penalty = lives_lost * self.LIFE_LOST_PENALTY

        self.total_score = self.sack_score + self.end_bonus + self.golden_score - self.lives_penalty

        # Rolling counters for each row
        self.rolling_sack = 0
        self.rolling_end = 0
        self.rolling_golden = 0
        self.rolling_penalty = 0
        self.rolling_total = 0

        # Current row being displayed (0-4, 5 = done)
        self.current_row = 0
        self.row_start_time = global_state.frame_time
        self.row_phase = 0  # 0=forward_anim, 1=rolling, 2=reverse_anim
        self.phase_start_time = global_state.frame_time

        # Sound state
        self.is_rolling = False
        self.score_counter_start = None

    def _hugo_forward_time(self):
        return len(ScoreboardResources.hugo_side) / self.HUGO_ANIM_FPS

    def process_events(self, phone_events: PhoneEvents):
        phase_time = global_state.frame_time - self.phase_start_time
        forward_time = self._hugo_forward_time()

        if self.current_row < 5:
            if self.row_phase == 0:  # Hugo forward animation
                if phase_time >= forward_time:
                    self.row_phase = 1
                    self.phase_start_time = global_state.frame_time
            elif self.row_phase == 1:  # Score rolling
                self._update_rolling()
                if self._is_row_complete():
                    self.row_phase = 2
                    self.phase_start_time = global_state.frame_time
            elif self.row_phase == 2:  # Hugo reverse animation
                if phase_time >= forward_time:
                    self.current_row += 1
                    self.row_phase = 0
                    self.phase_start_time = global_state.frame_time
                    self.row_start_time = global_state.frame_time

        # Handle sound (only during rolling phase)
        rolling_active = self.row_phase == 1 and self.current_row < 5 and self._any_rolling()
        if rolling_active and not self.is_rolling:
            self.context.scoreboard_score_counter_id = AudioHelper.play(
                ScoreboardResources.score_counter, self.context.audio_port
            )
            self.score_counter_start = global_state.frame_time
            self.is_rolling = True
        elif rolling_active and self.is_rolling and global_state.frame_time - self.score_counter_start >= 0.1:
            self.context.scoreboard_score_counter_id = AudioHelper.play(
                ScoreboardResources.score_counter, self.context.audio_port
            )
            self.score_counter_start = global_state.frame_time
        elif not rolling_active and self.is_rolling:
            AudioHelper.stop(self.context.scoreboard_score_counter_id, self.context.audio_port)
            self.is_rolling = False

        # Exit after showing total for a while
        if self.current_row >= 5 and phase_time > 2.0:
            self.context.forest_score = self.total_score
            self.end()

    def _is_row_complete(self):
        if self.current_row == 0:
            return self.rolling_sack >= self.sack_score
        elif self.current_row == 1:
            return self.rolling_end >= self.end_bonus
        elif self.current_row == 2:
            return self.rolling_golden >= self.golden_score
        elif self.current_row == 3:
            return self.rolling_penalty >= self.lives_penalty
        elif self.current_row == 4:
            return self.rolling_total >= self.total_score
        return True

    def _any_rolling(self):
        if self.current_row == 0 and self.rolling_sack < self.sack_score:
            return True
        if self.current_row == 1 and self.rolling_end < self.end_bonus:
            return True
        if self.current_row == 2 and self.rolling_golden < self.golden_score:
            return True
        if self.current_row == 3 and self.rolling_penalty < self.lives_penalty:
            return True
        if self.current_row == 4 and self.rolling_total < self.total_score:
            return True
        return False

    def _update_rolling(self):
        if self.current_row >= 0:
            self.rolling_sack = min(self.sack_score, self.rolling_sack + self.ROLL_SPEED)
        if self.current_row >= 1:
            self.rolling_end = min(self.end_bonus, self.rolling_end + self.ROLL_SPEED)
        if self.current_row >= 2:
            self.rolling_golden = min(self.golden_score, self.rolling_golden + self.ROLL_SPEED)
        if self.current_row >= 3:
            self.rolling_penalty = min(self.lives_penalty, self.rolling_penalty + self.ROLL_SPEED)
        if self.current_row >= 4:
            self.rolling_total = min(self.total_score, self.rolling_total + self.ROLL_SPEED)

    def render(self, screen):
        screen.blit(ScoreboardResources.background, (0, 0))

        self._render_hugo(screen)

        if self.current_row >= 0:
            self._render_row(screen, 0, ScoreboardResources.icon_sack, self.rolling_sack, False)
        if self.current_row >= 1:
            self._render_row(screen, 1, ScoreboardResources.icon_finish, self.rolling_end, False)
        if self.current_row >= 2:
            self._render_row(screen, 2, ScoreboardResources.icon_golden_sack, self.rolling_golden, False)
        if self.current_row >= 3:
            self._render_lives_lost_row(screen, 3)
        self._render_total_row(screen, 4)

    def _render_hugo(self, screen):
        if self.current_row >= 5:
            return

        progress = min(1.0, self.current_row / 4.0)
        hugo_y = self.HUGO_START_Y + (self.HUGO_END_Y - self.HUGO_START_Y) * progress

        phase_time = global_state.frame_time - self.phase_start_time
        num_frames = len(ScoreboardResources.hugo_side)

        if self.row_phase == 0:  # Forward animation
            frame_idx = min(int(phase_time * self.HUGO_ANIM_FPS), num_frames - 1)
        elif self.row_phase == 1:  # Rolling - hold last frame
            frame_idx = num_frames - 1
        elif self.row_phase == 2:  # Reverse animation
            frame_idx = max(num_frames - 1 - int(phase_time * self.HUGO_ANIM_FPS), 0)
        else:
            frame_idx = 0

        screen.blit(ScoreboardResources.hugo_side[frame_idx], (self.HUGO_X, int(hugo_y)))

    def _render_row(self, screen, row_index, icon, score, is_negative):
        y = self.FIRST_ROW_Y + row_index * self.ROW_HEIGHT

        screen.blit(icon, (self.ICON_X, y))
        self._render_score(screen, self.SCORE_X, y + 8, score, is_negative)

    def _render_lives_lost_row(self, screen, row_index):
        y = self.FIRST_ROW_Y + row_index * self.ROW_HEIGHT

        obstacles_hit = self.context.forest_obstacles_hit or []
        icon_x = self.ICON_X
        obstacle_icons = {
            1: ScoreboardResources.icon_obstacle_catapult,
            2: ScoreboardResources.icon_obstacle_trap,
            3: ScoreboardResources.icon_obstacle_rock,
            4: ScoreboardResources.icon_obstacle_branch,
        }

        for i, obstacle_type in enumerate(obstacles_hit[:3]):
            icon = obstacle_icons.get(obstacle_type)
            if icon:
                ix = icon_x + i * 28
                # Draw background square behind obstacle icon
                screen.blit(ScoreboardResources.icon_death_bg, (ix - 2, y - 1))
                screen.blit(icon, (ix, y))

        self._render_score(screen, self.SCORE_X, y + 8, self.rolling_penalty, is_negative=True)

    def _render_total_row(self, screen, row_index):
        y = self.FIRST_ROW_Y + row_index * self.ROW_HEIGHT
        screen.blit(ScoreboardResources.icon_total, (self.ICON_X-10, y+15))
        self._render_score(screen, self.SCORE_X, y + 8, self.rolling_total, is_negative=False)

    def _render_score(self, screen, x, y, score, is_negative=False):
        score = int(score)

        if is_negative and score > 0:
            screen.blit(ScoreboardResources.icon_negative, (x - 17, y))

        digits = []
        temp = abs(score)
        for _ in range(6):
            digits.insert(0, temp % 10)
            temp //= 10

        for i, digit in enumerate(digits):
            screen.blit(
                ScoreboardResources.score_font[digit],
                (x + i * self.SCORE_DIGIT_SPACING, y)
            )

    def end(self):
        if self.is_rolling:
            AudioHelper.stop(self.context.scoreboard_score_counter_id, self.context.audio_port)
            self.is_rolling = False
        self.ended = True
