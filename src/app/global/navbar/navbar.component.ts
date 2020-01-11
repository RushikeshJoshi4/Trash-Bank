import { Component, OnInit } from '@angular/core';
import { ActivatedRoute, Router, NavigationEnd } from '@angular/router';

@Component({
  selector: 'app-navbar',
  templateUrl: './navbar.component.html',
  styleUrls: ['./navbar.component.scss']
})
export class NavbarComponent implements OnInit {

  activeTab: number;
  constructor(private router: Router) {
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
        // console.log(re);
      }
    });
  }

  ngOnInit() {
  }

}
