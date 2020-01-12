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
    $('.odometer')[0].innerText = this.ego;
  }

  ngAfterViewInit() {
    setInterval(() => {
      this.ego++;
      $('.odometer')[0].innerText = this.ego;
    }, 7000);
  }

}
