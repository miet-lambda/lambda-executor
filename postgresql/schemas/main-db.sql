CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    login VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL,
    money_balance NUMERIC(30, 10) NOT NULL,
    token_version INT NOT NULL
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

CREATE TABLE projects_kv_storage (
    project_id INT NOT NULL,
    key VARCHAR(255) NOT NULL,
    value TEXT NOT NULL,
    PRIMARY KEY (project_id, key),
    FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE CASCADE
);
