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
'),
(4, '/v1/test/network/factorial', 1,
'
  local json = require("dkjson")
  local client = require("miet.http.client").get()
  local context = require("miet.http.context").get()

  local request = context:request()
  local outgoing_response = context:response()

  local incoming_n = tonumber(request["query"]["n"])

  if incoming_n == 0 or incoming_n == 1 then
    outgoing_response["body"] = {
      result = 1
    }
    return
  end

  local send_body = {
    method = "GET",
    url = "http://localhost:8080/v1/execute/lambda/4",
    query = {
      n = tostring(incoming_n - 1)
    }
  }

  local response, err = client:send("GET", "http://localhost:8080/v1/execute/lambda/4", {
    body = json.encode(send_body, { indent = true })
  })

  if err ~= nil then
    outgoing_response["status"] = 500
    outgoing_response["body"] = err
    return
  end

  local data, pos, err = json.decode(response["body"], 1, nil)
  if err ~= nil then
    outgoing_response["status"] = 500
    outgoing_response["body"] = err
    return
  end

  local inner_data, pos, err = json.decode(data["body"], 1, nil)
  if err ~= nil then
    outgoing_response["status"] = 500
    outgoing_response["body"] = err
    return
  end

  if data["status"] ~= 200 or inner_data["result"] == nil then
    outgoing_response["status"] = 500
    outgoing_response["body"] = data["body"]
    return
  end

  outgoing_response["body"] = {
    result = incoming_n * tonumber(inner_data["result"])
  }
'),
(5, '/v1/test/http/client', 1,
'
  local json = require("dkjson")
  local client = require("miet.http.client").get()
  local context = require("miet.http.context").get()

  local incoming_request = context:request()
  local outgoing_response = context:response()

  local auth_info = {
    login = "test",
    password = "password"
  }

  local response, err = client:post("$mockserver/users/data", {
    body = json.encode(auth_info, { indent = true }),
    headers = {
      Authorization = "basic"
    }
  })

  if err ~= nil then
    outgoing_response["status"] = 500
    outgoing_response["body"] = err
    return
  end

  local data, pos, err = json.decode(response["body"], 1, nil)
  if err ~= nil then
    outgoing_response["status"] = 500
    outgoing_response["body"] = err
    return
  end

  outgoing_response["body"] = response["body"]
'),
(6, '/v1/test/kv/storage', 1,
'
  local context = require("miet.http.context").get()
  local storage = require("miet.kv.storage").get()

  local incoming_request = context:request()
  local outgoing_response = context:response()

  local store_info = {
    name = "Test",
    age = 18,
    marks = { 4, 4, 5 }
  }

  local err = storage:store("info", store_info)
  if err ~= nil then
    outgoing_response["status"] = 500
    outgoing_response["body"] = err
    return
  end

  local restored_info, err = storage:get("info"):as_table()
  if err ~= nil then
    outgoing_response["status"] = 500
    outgoing_response["body"] = err
    return
  end

  outgoing_response["body"] = restored_info
');
