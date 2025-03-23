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
