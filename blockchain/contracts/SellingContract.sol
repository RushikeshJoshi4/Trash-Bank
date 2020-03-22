pragma solidity ^0.5.12;
pragma experimental ABIEncoderV2;


contract SellingContract {

    address owner;

    enum ItemType {PLASTIC, PAPER, METAL, GLASS}
    enum State {AWAITING_PAYMENT, AWAITING_DELIVERY, COMPLETE}

    struct Item {
        uint id;
        address owner;
        bytes32 title;
        uint weight;
        uint price;
        ItemType item_type;
        bytes32 image_path;
        bool forSale;

    }

    struct User {
        uint id;
        address acc;
        bytes32 uname;
        uint balance;
        bytes32 image_path;
    }

    // this is added only when an user initiates a Transaction
    struct Transaction {
        uint id;
        address buyer;
        address seller;
        uint item_id;
        State curr_state;
        bytes32 datetime; //added this
    }

    mapping (uint => Item) public items;
    mapping (uint => Item[]) public user_items;
    mapping (address => uint) public deposits; // for escrow
    mapping (address => uint) public users;
    mapping (uint => User) public id_to_users;
    mapping (uint => Transaction[]) public user_transactions;
    mapping (uint => Transaction) public id_to_transac;
    mapping (bytes32 => address) public uname_to_address;
    mapping (address => bytes32) public address_to_uname;
    mapping (uint => bytes32) public itemid_to_title; //add this


    uint public item_count;
    uint public transac_count;
    uint public user_count;

    event ItemOwnerChanged(
        uint index
    );

    event PlotPriceChanged(
        uint index,
        uint price
    );

    event PlotAvailabilityChanged(

        uint index,
        uint price,
        bool forSale
    );

    event UserBalanceChanged(
        uint user_id,
        uint balance
    );

    event TransactionUpdated(
        uint id,
        State state
    );

    event UserRegistered(
        uint id
    );

    uint id;
    bool forSale;
    uint weight;
    uint price;
    ItemType item_type;
    bytes32 image_path;


    constructor() public{
        owner = msg.sender;
        // for(uint i=0; i<12; i++){
        //     items[i] = Item(id,owner,true,40,40,ItemType.PLASTIC, "");
        // }
        item_count = 1;
        user_count = 1;
        transac_count = 1;

    }

    function signUp(bytes32 uname, address acc, uint bal) public{
        require(acc==msg.sender);
        users[acc] = user_count;
        uname_to_address[uname] = acc;
        id_to_users[user_count] = User(user_count, acc, uname, bal, "");
        address_to_uname[acc] = uname;
        user_count++;
        // emit UserRegistered(user_count-1);

    }

    function logIn() public view returns (uint) {
        return users[msg.sender];
    }

    function addItem(uint user_id, address owner, bytes32 title, uint weight, uint price, ItemType item_type, bytes32 image_path) public {
            require(msg.sender == owner);
            items[item_count] = Item(item_count,msg.sender,title,weight,price,item_type,image_path,true);
            itemid_to_title[item_count] = title;
            user_items[user_id].push(Item(item_count,msg.sender,title,weight,price,item_type,image_path,true));
            item_count++;

        }

        // function putItemUpForSale(uint id, uint price) public {
        //     Item storage item = items[id];
        //     require(msg.sender==item.owner && price > 0);
        //     item.forSale = true;
        //     item.price = price;
        //     emit ItemAvailabilityChanged(id, price, true);

        // }

        function buyItem(uint user_id, uint id, bytes32 datetime) public payable {
            Item storage item = items[id];
            require(msg.sender != item.owner && id_to_users[user_id].balance > item.price);
            deposits[msg.sender] += item.price;

            id_to_users[user_id].balance -= item.price;
            user_transactions[user_id].push(Transaction(transac_count,msg.sender, item.owner, id, State.AWAITING_DELIVERY,datetime));
            user_transactions[users[item.owner]].push(Transaction(transac_count,msg.sender, item.owner, id, State.AWAITING_DELIVERY,datetime));
            id_to_transac[transac_count] = Transaction(transac_count,msg.sender,item.owner,id,State.AWAITING_DELIVERY,datetime);
            item.forSale = false;
            transac_count++;

            emit UserBalanceChanged(user_id,id_to_users[user_id].balance);
        }

        function getTransactions(uint user_id) public view returns (Transaction[] memory){
            return user_transactions[user_id];
        }

        function getItemByUser(uint user_id) public view returns (Item[] memory){
            return user_items[user_id];
        }

        function confirmItemDeal(uint user_id, uint item_id, uint transac_id) public payable{

            // require(id_to_transac[transac_id].buyer == msg.sender && id_to_transac[transac_id].curr_state == State.AWAITING_DELIVERY);
            Item storage item = items[item_id];
            Transaction storage trans = id_to_transac[transac_id];
            item.owner = msg.sender;
            trans.curr_state = State.COMPLETE;
            deposits[msg.sender] -= item.price;
            // address payable seller = address(uint160(id_to_transac[transac_id].seller));
            // seller.transfer(item.price);
            User storage seller_user = id_to_users[users[trans.seller]];
            seller_user.balance += item.price;

            emit TransactionUpdated(transac_id,State.COMPLETE);
            emit ItemOwnerChanged(item_id);
        }

    //
    // function addPlot(uint user_id, bool forSale, uint price) public {
    //     Plot memory plot = Plot(plot_count,msg.sender,forSale,price);
    //     plots[plot_count] = plot;
    //     plot_count++;
    //     user_plots[user_id].push(plot);
    // }

    // function putPlotUpForSale(uint id, uint price) public {
    //     Plot storage plot = plots[id];
    //     require(msg.sender==plot.owner && price > 0);
    //     plot.forSale = true;
    //     plot.price = price;
    //     emit PlotAvailabilityChanged(id, price, true);
    //
    // }
    //
    // function buyPlot(uint user_id, uint id) public payable {
    //     Plot storage plot = plots[id];
    //     require(msg.sender != plot.owner && plot.forSale && id_to_users[user_id].balance > plot.price);
    //     deposits[msg.sender] += plot.price;
    //     id_to_users[user_id].balance -= plot.price;
    //     user_transactions[user_id].push(Transaction(transac_count,msg.sender, plot.owner, id, State.AWAITING_DELIVERY));
    //     id_to_transac[transac_count] = Transaction(transac_count,msg.sender,plot.owner,id,State.AWAITING_DELIVERY);
    //     plot.forSale = false;
    //     transac_count++;
    //     emit UserBalanceChanged(user_id,id_to_users[user_id].balance);
    //
    //
    // }
    //
    // function getTransactions(uint user_id) public view returns (Transaction[] memory){
    //     return user_transactions[user_id];
    //
    // }
    //
    // function getPlotByUser(uint user_id) public view returns (Plot[] memory){
    //     return user_plots[user_id];
    // }
    //
    // function confirmPlotDeal(uint user_id, uint plot_id, uint transac_id) public payable{
    //
    //     require(id_to_transac[transac_id].buyer == msg.sender && id_to_transac[transac_id].curr_state == State.AWAITING_DELIVERY);
    //     Plot storage plot = plots[plot_id];
    //     plot.owner = msg.sender;
    //     id_to_transac[transac_id].curr_state = State.COMPLETE;
    //     deposits[msg.sender] -= plot.price;
    //     address payable seller = address(uint160(id_to_transac[transac_id].seller));
    //     seller.transfer(plot.price);
    //     id_to_users[users[seller]].balance += plot.price;
    //     emit TransactionUpdated(transac_id,State.COMPLETE);
    //     emit PlotOwnerChanged(plot_id);
    // }


}
