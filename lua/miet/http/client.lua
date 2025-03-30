local miet_module = {}

local extended_http_client_object = {
  _impl = _G.miet_http_client
}

function extended_http_client_object:send(method, url, extra)
  return self._impl:send(method, url, extra)
end

function extended_http_client_object:delete(url, extra)
  return self._impl:send('DELETE', url, extra)
end

function extended_http_client_object:put(url, extra)
  return self._impl:send('PUT', url, extra)
end

function extended_http_client_object:patch(url, extra)
  return self._impl:send('PATCH', url, extra)
end

function extended_http_client_object:options(url, extra)
  return self._impl:send('OPTIONS', url, extra)
end

function extended_http_client_object:get(url, extra)
  return self._impl:send('GET', url, extra)
end

function extended_http_client_object:post(url, extra)
  return self._impl:send('POST', url, extra)
end

function extended_http_client_object:head(url, extra)
  return self._impl:send('HEAD', url, extra)
end

function miet_module.get()
  return extended_http_client_object
end

return miet_module
