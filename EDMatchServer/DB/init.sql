-- EDMatchServer Database Setup
-- Run: mysql -u root -p < init.sql

CREATE DATABASE IF NOT EXISTS eternal_dreams
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE eternal_dreams;

-- Account table
CREATE TABLE IF NOT EXISTS accounts (
    id          BIGINT AUTO_INCREMENT PRIMARY KEY,
    login_id    VARCHAR(64) UNIQUE NOT NULL,
    pw_hash     VARCHAR(256) NOT NULL,
    nickname    VARCHAR(32) UNIQUE NOT NULL,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Match history (Phase 8)
CREATE TABLE IF NOT EXISTS match_history (
    id          BIGINT AUTO_INCREMENT PRIMARY KEY,
    match_id    VARCHAR(64) NOT NULL,
    user_id     BIGINT NOT NULL,
    team_id     INT NOT NULL,
    result      ENUM('win','lose','draw') NOT NULL,
    played_at   DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES accounts(id)
);

-- Test accounts (password: 1234 -> SHA256 hash)
INSERT IGNORE INTO accounts (login_id, pw_hash, nickname) VALUES
    ('test1', '03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4', 'Player1'),
    ('test2', '03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4', 'Player2'),
    ('test3', '03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4', 'Player3');
