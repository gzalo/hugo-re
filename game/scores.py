import sqlite3

class Scores:
    DATABASE = "scores.db"

    def __init__(self):
        self.conn = sqlite3.connect(self.DATABASE)

        sql_create_scores_table = """ CREATE TABLE IF NOT EXISTS scores (
                                               id integer PRIMARY KEY,
                                               game_name text NOT NULL,
                                               player_name text NOT NULL,
                                               score integer NOT NULL
                                           ); """
        cursor = self.conn.cursor()
        cursor.execute(sql_create_scores_table)

    def insert_score(self, game_name, player_name, score):
        if player_name == "":
            return

        sql = ''' INSERT INTO scores(game_name, player_name, score)
                  VALUES(?,?,?) '''
        cursor = self.conn.cursor()
        cursor.execute(sql, (game_name, player_name, score))
        self.conn.commit()

    def get_top_scores(self, game_name, limit=10):
        sql = ''' SELECT player_name, score
                  FROM scores
                  WHERE game_name = ?
                  ORDER BY score DESC
                  LIMIT ? '''
        cursor = self.conn.cursor()
        cursor.execute(sql, (game_name, limit))
        return cursor.fetchall()

    def init_dummy_scores(self, game_options):
        for game_name in game_options:
            for score in range(0,5):
                self.insert_score(game_name, "Hugo", score)