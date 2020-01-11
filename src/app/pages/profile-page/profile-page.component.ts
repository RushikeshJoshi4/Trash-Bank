import { Component, OnInit } from '@angular/core';
import { Transaction, TransactionStatus } from 'src/app/models/transaction';
import { TRANSACS } from 'src/app/models/mockdata/mock_transac';

@Component({
  selector: 'app-profile-page',
  templateUrl: './profile-page.component.html',
  styleUrls: ['./profile-page.component.scss']
})
export class ProfilePageComponent implements OnInit {

  transactions: Transaction[];
  constructor() {
    this.transactions = [];
  }

  ngOnInit() {
    this.transactions = TRANSACS;
  }

  generateClassObj(t: Transaction) {
    const obj = {
      'text-success': t.status === TransactionStatus.COMPLETED,
      'text-warning': t.status === TransactionStatus.PENDING,
    };
    return obj;
  }

}
