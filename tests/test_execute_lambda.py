import pytest

import json

from testsuite.databases import pgsql


def assert_response(response, expected_code):
    content_type = 'application/json; charset=utf-8'
    assert response.status == 200
    assert response.json()['status'] == expected_code, response.json()
    assert response.headers['Content-Type'] == content_type


@pytest.mark.pgsql('main-db', files=['scripts.sql'])
async def test_execute_lambda_nonexist_script(service_client):
    response = await service_client.post(
        '/v1/execute/lambda/123',
        json={
            'method': 'GET',
            'url': '/unknown/script'
        }
    )
    assert response.status == 404


@pytest.mark.pgsql('main-db', files=['scripts.sql'])
async def test_execute_lambda_without_context(service_client):
    response = await service_client.post(
        '/v1/execute/lambda/1',  # /v1/test/without/context
        json={
            'method': 'GET',
            'url': '/v1/test/without/context'
        }
    )
    assert_response(response, 200)


@pytest.mark.pgsql('main-db', files=['scripts.sql'])
async def test_execute_lambda_factorial(service_client):
    response = await service_client.post(
        '/v1/execute/lambda/2',  # /v1/test/factorial
        json={
            'method': 'GET',
            'url': '/v1/test/factorial',
            'query': {
                'n': '5'
            }
        }
    )
    assert_response(response, 200)
    body = json.loads(response.json()['body'])
    assert body['result'] == 120


@pytest.mark.pgsql('main-db', files=['scripts.sql'])
async def test_execute_lambda_json(service_client):
    response = await service_client.post(
        '/v1/execute/lambda/3',  # /v1/test/json
        json={
            'method': 'POST',
            'url': '/v1/test/json',
            'headers': {
                'Content-Type': 'application/json'
            },
            'body': '{"name": "Test"}'
        }
    )
    assert_response(response, 200)
    assert response.json()['body'] == 'Hello, Test!'

    response = await service_client.post(
        '/v1/execute/lambda/3',  # /v1/test/json
        json={
            'method': 'POST',
            'url': '/v1/test/json',
            'body': '{"name": "Test"}'
        }
    )
    assert_response(response, 400)
    body = json.loads(response.json()['body'])
    assert body['message'] == 'Expected json format'

    response = await service_client.post(
        '/v1/execute/lambda/3',  # /v1/test/json
        json={
            'method': 'POST',
            'url': '/v1/test/json',
            'headers': {
                'Content-Type': 'application/json'
            }
        }
    )
    assert_response(response, 400)
    body = json.loads(response.json()['body'])
    assert body['message'] == 'Expected name field'


@pytest.mark.pgsql('main-db', files=['scripts.sql'])
async def test_execute_lambda_http_client(service_client, mockserver):
    @mockserver.json_handler('/users/data')
    def mock_external_service(request):
        assert request.method == 'POST'
        assert request.query['key'] == 'value'
        assert request.headers['Authorization'] == 'basic'
        assert request.json['login'] == 'test'
        assert request.json['password'] == 'password'
        return {
            'name': 'Test',
            'age': 18,
        }

    response = await service_client.post(
        '/v1/execute/lambda/5',  # /v1/test/http/client
        json={
            'method': 'GET',
            'url': '/v1/test/http/client'
        }
    )
    assert_response(response, 200)
    body = json.loads(response.json()['body'])
    assert body['name'] == 'Test'
    assert body['age'] == 18


@pytest.mark.pgsql('main-db', files=['scripts.sql'])
async def test_execute_lambda_kv_storage(service_client):
    response = await service_client.post(
        '/v1/execute/lambda/6',  # /v1/test/kv/storage
        json={
            'method': 'GET',
            'url': '/v1/test/kv/storage'
        }
    )
    assert_response(response, 200)
    body = json.loads(response.json()['body'])
    assert body['name'] == 'Test'
    assert body['age'] == 18
    assert body['marks'] == [4, 4, 5]
