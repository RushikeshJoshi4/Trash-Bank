import { Component, OnInit } from '@angular/core';
import { ItemType } from 'src/app/models/item';
import { ItemService } from 'src/app/services/item.service';
import { LoginService } from 'src/app/services/login.service';

@Component({
  selector: 'app-sell-page',
  templateUrl: './sell-page.component.html',
  styleUrls: ['./sell-page.component.scss']
})
export class SellPageComponent implements OnInit {

  // selectedItemType: ItemType;
  uploadLabel: string;
  formdata: any;
  ItemType = ItemType;

  constructor(private itemService: ItemService, private loginService: LoginService) {
    this.uploadLabel = 'Upload photo of product to predict fields below!';
  }

  ngOnInit() {
    this.formdata = {
      user_id: this.loginService.user.ID,
      address: this.loginService.user.address,
      title: '',
      weight: 0,
      price: 0,
      item_type: undefined,
      image: undefined,
    };
  }

  pickImage(event) {
    const file: File = event.target.files[0];
    this.uploadLabel = file.name;
    this.formdata.image = file;
  }

  submit() {
    console.log(this.formdata);
    this.itemService.addItem(this.formdata).subscribe(addresp => {
      console.log(addresp);
    });
  }

  test(x) {
    console.log(x);
  }

}
