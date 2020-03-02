import { Component, OnInit } from '@angular/core';
import { ActivatedRoute, Router, NavigationEnd } from '@angular/router';
import { UtilityService } from 'src/app/services/utility.service';
import { LoginService } from 'src/app/services/login.service';
import { User } from 'src/app/models/user';
declare var $: any;

@Component({
  selector: 'app-navbar',
  templateUrl: './navbar.component.html',
  styleUrls: ['./navbar.component.scss']
})
export class NavbarComponent implements OnInit {

  activeTab: number;
  constructor(private router: Router, private utility: UtilityService, private login: LoginService) {
    this.router.events.subscribe(re => {
      if (re instanceof NavigationEnd) {
        const currentPath: string = re.url;
        if (currentPath === '/home') {
          this.activeTab = 1;
        } else if (currentPath === '/buy') {
          this.activeTab = 2;
        } else if (currentPath === '/sell') {
          this.activeTab = 3;
        } else if (currentPath === '/profile') {
          this.activeTab = 4;
        }
      }
    });
  }

  ngOnInit() {
    // this.utility.displayLoading('Loading user data...');

    // this.login.getUserDetails().subscribe(resp => {
    //   this.login.user = new User(resp);
    //   this.utility.hideLoading();
    //   console.log('user', this.login.user);
    // });
  }

}
