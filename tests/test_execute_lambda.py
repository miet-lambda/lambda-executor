import pytest

import json

from testsuite.databases import pgsql


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
    assert response.status == 200
    assert response.json()['status'] == 200


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
    assert response.status == 200
    assert response.json()['status'] == 200
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
    assert response.status == 200
    assert response.json()['status'] == 200
    assert response.json()['body'] == 'Hello, Test!'

    response = await service_client.post(
        '/v1/execute/lambda/3',  # /v1/test/json
        json={
            'method': 'POST',
            'url': '/v1/test/json',
            'body': '{"name": "Test"}'
        }
    )
    assert response.status == 200
    assert response.json()['status'] == 400
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
    assert response.status == 200
    assert response.json()['status'] == 400
    body = json.loads(response.json()['body'])
    assert body['message'] == 'Expected name field'


@pytest.mark.pgsql('main-db', files=['scripts.sql'])
async def test_execute_lambda_http_client(service_client, mockserver):
    @mockserver.json_handler('/users/data')
    def mock_external_service(request):
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
    assert response.status == 200
    assert response.json()['status'] == 200, response.json()
    body = json.loads(response.json()['body'])
    assert body['name'] == 'Test'
    assert body['age'] == 18
