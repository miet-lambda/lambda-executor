INSERT INTO users (id, login, password_hash, salt, money_balance) VALUES
(1, 'test-scripts', 'password-hash', 'sault', 100);

INSERT INTO projects (id, name, owner_id) VALUES
(1, 'test-project', 1);

INSERT INTO scripts (id, path, parent_project_id, source_code) VALUES
(1, '/v1/test/without/context', 1,
'
  local a = 5
  local b = 6
  local c = a + b
'),
(2, '/v1/test/factorial', 1,
'
  local function factorial(n)
    if n == 0 or n == 1 then
      return 1
    end
    return n * factorial(n - 1)
  end

  local context = require("miet.http.context").get()
  local request = context:request()
  local response = context:response()

  local n = tonumber(request["query"]["n"])
  response["body"] = {
    result = factorial(n)
  }
'),
(3, '/v1/test/json', 1,
'
  local json = require("dkjson")
  local context = require("miet.http.context").get()
  local request = context:request()
  local response = context:response()

  if request["headers"]["Content-Type"] == nil or request["headers"]["Content-Type"] ~= "application/json" then
    response["status"] = 400
    response["body"] = {
      message = "Expected json format"
    }
    return
  end

  local data, pos, err = json.decode(request["body"], 1, nil)
  if data == nil or data["name"] == nil then
    response["status"] = 400
    response["body"] = {
      message = "Expected name field"
    }
    return
  end

  response["body"] = "Hello, " .. data["name"] .. "!"
');
