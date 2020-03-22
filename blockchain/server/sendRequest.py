
import requests


def confirm():
    url = "http://127.0.0.1:5000/confirm"

    payload = "{\"user_id\":2,\"item_id\":1, \"transac_id\":1, \"address\":\"0x06e532B68c27108b10314CDd4D6Dc77dE2e7DB01\"}"
    headers = {
        'content-type': "application/json",
        'cache-control': "no-cache",
        'postman-token': "c75ed62c-52e0-1d3b-8459-e909c0b8faf9"
        }

    response = requests.request("POST", url, data=payload, headers=headers)

    print(response.text)

def add(payload):
    url = "http://127.0.0.1:5000/add"

    headers = {
        'content-type': "application/json",
        'cache-control': "no-cache",
        'postman-token': "dd998de3-dd30-5015-6430-9254a9fbca05"
        }

    response = requests.request("POST", url, data=payload, headers=headers)

    print(response.text)


def items():
    url = "http://127.0.0.1:5000/items"

    payload = "{\"user_id\":1,\"address\":\"0x1C0904ac0eF6D7f08C73d95180a7a68cf52a31f3\"}"
    headers = {
        'content-type': "application/json",
        'cache-control': "no-cache",
        'postman-token': "069a47d8-4958-0854-20fb-a12305c9252f"
        }

    response = requests.request("POST", url, data=payload, headers=headers)

    print(response.text)


def buy():
    url = "http://127.0.0.1:5000/buy"

    payload = "{\"user_id\":2,\"item_id\":1, \"address\":\"0x06e532B68c27108b10314CDd4D6Dc77dE2e7DB01\"}"
    headers = {
        'content-type': "application/json",
        'cache-control': "no-cache",
        'postman-token': "be9e397a-6cac-f20e-c61c-5909b7268a5b"
        }

    response = requests.request("POST", url, data=payload, headers=headers)

    print(response.text)

def transaction():
    url = "http://127.0.0.1:5000/transaction"

    payload = "{\"user_id\":2,\"address\": \"0x06e532B68c27108b10314CDd4D6Dc77dE2e7DB01\"}"
    headers = {
        'content-type': "application/json",
        'cache-control': "no-cache",
        'postman-token': "b70a8eb7-2230-e32e-681b-a4960b754278"
        }

    response = requests.request("POST", url, data=payload, headers=headers)

    print(response.text)

# payload = "{\"user_id\":1, \"address\":\"0x1C0904ac0eF6D7f08C73d95180a7a68cf52a31f3\",\n\"title\":\"Boxes For Sale\", \"weight\":4000, \"price\":8, \"item_type\":1, \"image_path\":\"cardboard.jpg\"}"
# add(payload)
# payload = "{\"user_id\":1, \"address\":\"0x1C0904ac0eF6D7f08C73d95180a7a68cf52a31f3\",\n\"title\":\"Plastic Bottles\", \"weight\":1800, \"price\":2, \"item_type\":0, \"image_path\":\"plastic.jpg\"}"
# add(payload)
# payload = "{\"user_id\":1, \"address\":\"0x1C0904ac0eF6D7f08C73d95180a7a68cf52a31f3\",\n\"title\":\"Metal Grill\", \"weight\":12000, \"price\":18, \"item_type\":2, \"image_path\":\"metal.jpg\"}"
# add(payload)


payload = "{\"user_id\":2, \"address\":\"0x06e532B68c27108b10314CDd4D6Dc77dE2e7DB01\",\n\"title\":\"Cardboard junk\", \"weight\":38246, \"price\":12, \"item_type\":1, \"image_path\":\"cardboard12.jpg\"}"
add(payload)
payload = "{\"user_id\":2, \"address\":\"0x06e532B68c27108b10314CDd4D6Dc77dE2e7DB01\",\n\"title\":\"Plastic bags for sale\", \"weight\":12234, \"price\":5, \"item_type\":0, \"image_path\":\"plastic2.jpg\"}"
add(payload)
payload = "{\"user_id\":3, \"address\":\"0xFE5147B922aE044C31cD29c778b2B7e2B9b8890A\",\n\"title\":\"Metal car part\", \"weight\":23000, \"price\":28, \"item_type\":2, \"image_path\":\"metal2.jpg\"}"
add(payload)
payload = "{\"user_id\":3, \"address\":\"0xFE5147B922aE044C31cD29c778b2B7e2B9b8890A\",\n\"title\":\"Glass bottles\", \"weight\":10034, \"price\":1, \"item_type\":3, \"image_path\":\"glass.jpg\"}"
add(payload)
payload = "{\"user_id\":3, \"address\":\"0xFE5147B922aE044C31cD29c778b2B7e2B9b8890A\",\n\"title\":\"Selling metal scrapings \", \"weight\":42542, \"price\":43, \"item_type\":2, \"image_path\":\"metal3.jpg\"}"
add(payload)
# items()
# buy()
# transaction()
