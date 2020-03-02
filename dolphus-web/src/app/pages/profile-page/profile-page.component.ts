import { Component, OnInit } from '@angular/core';
import { Transaction, TransactionStatus } from 'src/app/models/transaction';
import { TRANSACS } from 'src/app/models/mockdata/mock_transac';
import { User } from 'src/app/models/user';
import { LoginService } from 'src/app/services/login.service';
import { UtilityService } from 'src/app/services/utility.service';
import { TransactionService } from 'src/app/services/transaction.service';

declare var $: any;

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
    // while (!this.login.user) { }
    this.login.getUserDetails().subscribe(uresp => {
      if (uresp instanceof User) {
        this.currentUser = uresp;
      } else {
        this.login.user = new User(uresp);
        this.currentUser = this.login.user;
      }
      const dp: string = 'url("../../../assets/users/' + this.currentUser.name + '.jpg")';
      console.log(dp);
      $($('#dp')[0]).css('background-image', dp);

      this.transac.getTransactionsOfUser(this.currentUser).subscribe(tresp => {
        console.log('transactions', tresp);
        this.transactions = Transaction.parseTransacArr(tresp.data);
        this.forSaleData = tresp.items_for_sale;
        this.utility.hideLoading(1000);
      });
    });


    // this.utility.displayLoading('Loading transactions...');
    // this.login.getUserDetails().subscribe(uresp => {
    //   this.login.user = new User(uresp);
    //   this.currentUser = this.login.user;
    //   this.transac.getTransactionsOfUser(this.currentUser).subscribe(tresp => {
    //     console.log(tresp);
    //     this.transactions = Transaction.parseTransacArr(tresp.data);
    //     this.forSaleData = tresp.items_for_sale;
    //     this.utility.hideLoading(1000);
    //   });
    // });

    // this.transactions = TRANSACS;
  }

  confirmDelivery(itemID: number, transacID: number) {
    this.utility.displayLoading();
    this.transac.confirmPurchase(this.currentUser, itemID, transacID).subscribe(cresp => {
      console.log('confirm', cresp);
      this.transac.getTransactionsOfUser(this.currentUser).subscribe(tresp => {
        console.log('transactions', tresp);
        this.transactions = Transaction.parseTransacArr(tresp.data);
        this.forSaleData = tresp.items_for_sale;
        this.utility.hideLoading(1000);
      });
    });
  }

  generateClassObj(t: Transaction) {
    const obj = {
      'text-success': t.status === TransactionStatus.COMPLETED,
      'text-warning': t.status === TransactionStatus.PENDING,
    };
    return obj;
  }

}
