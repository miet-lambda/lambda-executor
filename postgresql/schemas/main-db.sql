CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    login VARCHAR(255) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    money_balance NUMERIC(30, 10) NOT NULL,
    token_version INT NOT NULL
);

CREATE TABLE projects (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    owner_id INT NOT NULL,
    FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE,
    UNIQUE (name)
);

CREATE TABLE scripts (
    id SERIAL PRIMARY KEY,
    path VARCHAR(255) NOT NULL,
    parent_project_id INT,
    source_code TEXT NOT NULL,
    FOREIGN KEY (parent_project_id) REFERENCES projects(id) ON DELETE CASCADE,
    UNIQUE (parent_project_id, path)
);

CREATE TABLE projects_kv_storage (
    project_id INT NOT NULL,
    key VARCHAR(255) NOT NULL,
    value TEXT NOT NULL,
    PRIMARY KEY (project_id, key),
    FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE CASCADE
);

CREATE OR REPLACE FUNCTION decrease_user_balance(
    p_user_id INT,
    p_amount NUMERIC(30, 10)
) RETURNS BOOLEAN AS $$
DECLARE
    v_sufficient_funds BOOLEAN;
BEGIN
    SELECT money_balance >= p_amount INTO v_sufficient_funds
    FROM users
    WHERE id = p_user_id
    FOR UPDATE;
    
    IF NOT FOUND OR NOT v_sufficient_funds THEN
        RETURN FALSE;
    END IF;
    
    UPDATE users
    SET money_balance = money_balance - p_amount
    WHERE id = p_user_id;
    
    RETURN TRUE;
END;
$$ LANGUAGE plpgsql;
