export enum TransactionStatus {
    PENDING,
    COMPLETED
}

export class Transaction {
    ID: number;
    buyerID: number;
    sellerID: number;
    itemID: number;
    status: TransactionStatus;
}
