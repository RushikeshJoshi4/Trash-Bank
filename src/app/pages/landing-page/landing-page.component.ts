import { Component, OnInit, AfterViewInit } from '@angular/core';
declare var $: any;

@Component({
  selector: 'app-landing-page',
  templateUrl: './landing-page.component.html',
  styleUrls: ['./landing-page.component.scss']
})
export class LandingPageComponent implements OnInit, AfterViewInit {

  ego: number;
  constructor() {
    this.ego = 1001;
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
  }

}
