
from web3 import Web3
import flask
from flask import request, jsonify
import json
from flask_cors import CORS
from datetime import datetime




app = flask.Flask(__name__)
app.config["DEBUG"] = True
CORS(app)

ganache_url = "http://127.0.0.1:7545"
web3 = Web3(Web3.HTTPProvider(ganache_url))
BASE = "http://169.231.102.113:5000/static/images/"


with open('contract.json', 'r') as myfile:
    data=myfile.read()

abi = json.loads(data)["abi"]
# print(abi)
address = web3.toChecksumAddress("0x9E67F195012BF853443DCDD24FF25cddD446E99d")

def bytes_to_string(data):
    data = data.hex().rstrip("0")
    if len(data) % 2 != 0:
        data = data + '0'
    data = bytes.fromhex(data).decode('utf8')
    return data

def item_type_to_name(i_type):
    item_types = {0:"Plastic", 1:"Paper", 2:"Metal", 3:"Glass"}
    return  item_types[i_type]

def item_name_to_type(i_name):
    item_types = {"Plastic":0, "Paper":1, "Metal":2, "Glass":3}
    return  item_types[i_name]

def status(s):
    if(s==1):
        return "Awaiting Delivery"
    else:
        return "Complete"

@app.route('/test', methods=['GET'])
def test():
    web3.eth.defaultAccount = web3.eth.accounts[1]
    uname = "Buyer"
    contract = web3.eth.contract(address = address, abi = abi)

    u_add = contract.functions.users(web3.toChecksumAddress(web3.eth.accounts[1])).call()
    print(contract.functions.id_to_users(5).call())
    print(u_add)
    return "anki"

@app.route('/signUp', methods = ['GET'])
def signUp():
    web3 = Web3(Web3.HTTPProvider(ganache_url))

    uname = "Erica"
    web3.eth.defaultAccount = web3.eth.accounts[0]
    print(web3.eth.defaultAccount)
    contract = web3.eth.contract(address = address, abi = abi)
    tx_hash = contract.functions.signUp(uname.encode('utf-8'),web3.toChecksumAddress(web3.eth.accounts[0]),120).transact()
    web3.eth.waitForTransactionReceipt(tx_hash)

    uname = "Shiv"
    web3.eth.defaultAccount = web3.eth.accounts[1]
    print(web3.eth.defaultAccount)
    contract = web3.eth.contract(address = address, abi = abi)
    tx_hash = contract.functions.signUp(uname.encode('utf-8'),web3.toChecksumAddress(web3.eth.accounts[1]),232).transact()
    web3.eth.waitForTransactionReceipt(tx_hash)


    uname = "Kiara"
    web3.eth.defaultAccount = web3.eth.accounts[2]
    print(web3.eth.defaultAccount)
    contract = web3.eth.contract(address = address, abi = abi)
    tx_hash = contract.functions.signUp(uname.encode('utf-8'),web3.toChecksumAddress(web3.eth.accounts[2]),100).transact()
    web3.eth.waitForTransactionReceipt(tx_hash)


    uname = "Tesla Corp"
    web3.eth.defaultAccount = web3.eth.accounts[3]
    print(web3.eth.defaultAccount)
    contract = web3.eth.contract(address = address, abi = abi)
    tx_hash = contract.functions.signUp(uname.encode('utf-8'),web3.toChecksumAddress(web3.eth.accounts[3]),5231).transact()
    web3.eth.waitForTransactionReceipt(tx_hash)

    uname = "Logitech"
    web3.eth.defaultAccount = web3.eth.accounts[4]
    print(web3.eth.defaultAccount)
    contract = web3.eth.contract(address = address, abi = abi)
    tx_hash = contract.functions.signUp(uname.encode('utf-8'),web3.toChecksumAddress(web3.eth.accounts[4]),2000).transact()
    web3.eth.waitForTransactionReceipt(tx_hash)

    return tx_hash

'''returns user details'''

@app.route('/', methods=['POST'])
def login():
    data = request.json ##username
    uname = data['uname']
    u_add = data['address']
    contract = web3.eth.contract(address = address, abi = abi)
    if(u_add==-1):
        u_add = contract.functions.uname_to_address(uname.encode('utf-8')).call()

    #set the address
    web3.eth.defaultAccount = u_add
    ##login and get the details of the users
    user_id = contract.functions.logIn().call()
    uid, u_add, uname, balance, image_path = contract.functions.id_to_users(user_id).call()
    uname = bytes_to_string(uname)
    image_path = bytes_to_string(image_path)
    return_data = {"uname":uname,"uid":uid,"address":u_add,"balance":balance,"image_path":image_path}
    # print(web3.eth.accounts)
    return return_data


@app.route('/add',methods=['POST'])
def addItem():
    data= request.form
    file_obj = request.files['image']
    data = request.json
    user_id= int(data['user_id'])
    acc = data['address']
    title = data['title']
    weight = int(data['weight'])
    price = int(data['price'])
    item_type = item_name_to_type(data['item_type'])
    # image_path = "http://169.231.102.113:5000/static/images"
    image_path = file_obj.filename
    # image_path = data['image_path']

    print(image_path)
    file_obj.save("static/images/"+image_path)
    print(user_id)
    print(acc)

    print(title)

    print(weight)

    print(price)
    print(item_type)
    print(type(web3.toChecksumAddress(acc)))
    web3.eth.defaultAccount = acc
    contract = web3.eth.contract(address = address, abi = abi)
    tx_hash = contract.functions.addItem(user_id,web3.toChecksumAddress(acc),title.encode('utf-8'),weight,price,item_type,image_path.encode('utf-8')).transact()
    web3.eth.waitForTransactionReceipt(tx_hash)
    return {"success":True}


@app.route('/transaction',methods=['POST'])
def getTransactions():
    data = request.json
    user_id= data['user_id']
    acc = data['address']
    web3.eth.defaultAccount = acc
    contract = web3.eth.contract(address = address, abi = abi)

    ## items for sale
    total_items = contract.functions.item_count().call()
    items_for_sale = []
    for i in range(1,total_items):
        d = contract.functions.items(i).call()
        if(d[1]==acc and d[7]==True):
            temp = {}
            temp["item_id"] = d[0]
            temp["item_owner"] = bytes_to_string(contract.functions.address_to_uname(d[1]).call())
            temp['item_title'] = bytes_to_string(d[2])
            temp['item_weight'] = d[3]
            temp['item_price'] = d[4]
            temp['item_type'] = item_type_to_name(d[5])
            temp['item_path'] = BASE+bytes_to_string(d[6])
            temp["status"] ="For Sale"
            temp["date"] = "01/11/2019"
            items_for_sale.append(temp)

    transactions = contract.functions.getTransactions(user_id).call()
    if(len(transactions)==0):
        return {"items_for_sale":items_for_sale, "data":[]}
    print(contract.functions.id_to_users(user_id).call()[3])
    print(transactions)
    seller_id = contract.functions.users(transactions[0][2]).call()
    print(contract.functions.id_to_users(seller_id).call()[3])
    return_data = []
    for t in transactions:
        temp = {}
        temp["transac_id"] = t[0]
        t = contract.functions.id_to_transac(t[0]).call()
        temp["buyer"] =  bytes_to_string(contract.functions.address_to_uname(t[1]).call())
        temp["seller"] = bytes_to_string(contract.functions.address_to_uname(t[2]).call())
        temp["item"] = {}
        temp["item"]["item_id"] = t[3]
        item = contract.functions.items(t[3]).call()
        temp["item"]["item_weight"] = item[3]
        temp["item"]["item_price"] = item[4]
        temp["item"]["item_owner"] =  bytes_to_string(contract.functions.address_to_uname(item[1]).call())
        temp["item"]["item_path"] = BASE+bytes_to_string(item[6])
        temp["item"]["item_title"] = bytes_to_string(contract.functions.itemid_to_title(t[3]).call())
        temp["item"]["item_type"] = item_type_to_name(contract.functions.items(t[3]).call()[5])
        temp["date"] = bytes_to_string(t[5])
        temp["status"] = status(t[4])
        if(acc==t[2]):
            temp["is_seller"] = 1
        else:
            temp["is_seller"] = 0
        return_data.append(temp)

    return {"items_for_sale":items_for_sale ,"data":return_data}

# @app.route("/addItem", methods['POST'])
@app.route('/items',methods=['POST'])
def getItems():
    data = request.json
    user_id = data['user_id']
    acc = data['address']
    web3.eth.defaultAccount = acc
    contract = web3.eth.contract(address = address, abi = abi)
    total_items = contract.functions.item_count().call()
    return_data = []
    for i in range(1,total_items):
        d = contract.functions.items(i).call()
        if(d[1]!=acc and d[-1]==True):
            temp = {}
            temp["item_id"] = d[0]
            temp["item_owner"] = bytes_to_string(contract.functions.address_to_uname(d[1]).call())
            temp['item_title'] = bytes_to_string(d[2])
            temp['item_weight'] = d[3]
            temp['item_price'] = d[4]
            temp['item_type'] = item_type_to_name(d[5])
            temp['item_path'] = BASE+bytes_to_string(d[6])
            return_data.append(temp)

    return {"data":return_data}

@app.route('/buy',methods=['POST'])
def buyItem():
    data = request.json
    user_id = data['user_id']
    item_id = data['item_id']
    acc = data['address']
    dt=datetime.now().strftime("%m/%d/%Y")
    web3.eth.defaultAccount = acc
    contract = web3.eth.contract(address = address, abi = abi)
    tx_hash = contract.functions.buyItem(user_id,item_id,dt.encode('utf-8')).transact()
    web3.eth.waitForTransactionReceipt(tx_hash)
    return {"success":True}

@app.route('/confirm',methods=['POST'])
def confirmItemDeal():
    data = request.json
    user_id = data['user_id']
    item_id = data['item_id']
    transac_id = data['transac_id']
    acc = data['address']
    web3.eth.defaultAccount = acc
    contract = web3.eth.contract(address = address, abi = abi)
    tx_hash = contract.functions.confirmItemDeal(user_id, item_id, transac_id).transact()
    web3.eth.waitForTransactionReceipt(tx_hash)
    return {"success":True}


app.run(host='0.0.0.0')
