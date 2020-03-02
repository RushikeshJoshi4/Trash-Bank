import { Component, OnInit, AfterViewInit } from '@angular/core';
import { LoginService } from 'src/app/services/login.service';
declare var $: any;

@Component({
  selector: 'app-landing-page',
  templateUrl: './landing-page.component.html',
  styleUrls: ['./landing-page.component.scss']
})
export class LandingPageComponent implements OnInit, AfterViewInit {

  ego: number;
  username: string;
  constructor(private login: LoginService) {
    this.ego = 1001;
    this.username = '';
  }

  ngOnInit() {
    if ($('.odometer')[0]) {
      $('.odometer')[0].innerText = this.ego;
    }
  }

  ngAfterViewInit() {
    setInterval(() => {
      if ($('.odometer')[0]) {
        this.ego++;
        $('.odometer')[0].innerText = this.ego;
      }
    }, 5000);

    $('#user-modal').modal('show');
  }

  signin() {
    localStorage.setItem('uname', this.username);
    console.log(this.username, localStorage.getItem('uname'));
    this.login.getUserDetails(this.username).subscribe(resp => {
      console.log('landing sign in', resp);
    });
  }

}
