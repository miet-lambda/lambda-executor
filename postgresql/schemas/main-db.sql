CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    login VARCHAR(255) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    salt VARCHAR(255) NOT NULL,
    money_balance MONEY NOT NULL
);

CREATE TABLE tokens (
    token VARCHAR(255) PRIMARY KEY,
    user_id INT NOT NULL,
    expiration_time BIGINT NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE projects (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    owner_id INT NOT NULL,
    FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE scripts (
    id SERIAL PRIMARY KEY,
    path VARCHAR(255) NOT NULL,
    parent_project_id INT NOT NULL,
    source_code TEXT NOT NULL,
    FOREIGN KEY (parent_project_id) REFERENCES projects(id) ON DELETE CASCADE
);

CREATE TABLE active_runner_instances (
    ip_address VARCHAR(255) NOT NULL,
    port SMALLINT NOT NULL,
    PRIMARY KEY (ip_address, port)
);
