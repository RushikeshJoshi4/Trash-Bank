export class User {
    ID: number;
    address: string;
    name: string;
    balance: number;
    imagePath: string;

    constructor(u) {
        this.ID = u.uid;
        this.address = u.address;
        this.name = u.uname;
        this.balance = u.balance;
        this.imagePath = u.image_path;
    }
}
