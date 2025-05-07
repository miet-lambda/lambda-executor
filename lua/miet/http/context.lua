local miet_module = {}

local extended_http_context_object = {}

function extended_http_context_object:request()
  return miet_http_context_incoming_request
end

function extended_http_context_object:response()
  return miet_http_context_outgoing_response
end

function miet_module.get()
  return extended_http_context_object
end

return miet_module
