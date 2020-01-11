import { Component, OnInit } from '@angular/core';
import { Transaction, TransactionStatus } from 'src/app/models/transaction';
import { TRANSACS } from 'src/app/models/mockdata/mock_transac';
import { User } from 'src/app/models/user';
import { LoginService } from 'src/app/services/login.service';
import { UtilityService } from 'src/app/services/utility.service';
import { TransactionService } from 'src/app/services/transaction.service';

@Component({
  selector: 'app-profile-page',
  templateUrl: './profile-page.component.html',
  styleUrls: ['./profile-page.component.scss']
})
export class ProfilePageComponent implements OnInit {

  currentUser: User;
  transactions: Transaction[];
  forSaleData: any[];
  constructor(private login: LoginService, private transac: TransactionService, private utility: UtilityService) {
    this.transactions = [];
  }

  ngOnInit() {
    this.utility.displayLoading('Loading transactions...');
    this.login.getUserDetails().subscribe(uresp => {
      this.login.user = new User(uresp);
      this.currentUser = this.login.user;
      this.transac.getTransactionsOfUser(this.currentUser).subscribe(tresp => {
        console.log(tresp);
        this.transactions = Transaction.parseTransacArr(tresp.data);
        this.forSaleData = tresp.items_for_sale;
        this.utility.hideLoading(1000);
      });
    });

    // this.transactions = TRANSACS;
  }

  generateClassObj(t: Transaction) {
    const obj = {
      'text-success': t.status === TransactionStatus.COMPLETED,
      'text-warning': t.status === TransactionStatus.PENDING,
    };
    return obj;
  }

}
