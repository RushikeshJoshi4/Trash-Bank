import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { HttpClientModule } from '@angular/common/http';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { GlobalModule } from './global/global.module';
import { PagesModule } from './pages/pages.module';
import { ModelsModule } from './models/models.module';

@NgModule({
  declarations: [
    AppComponent
  ],
  imports: [
    BrowserModule,
    AppRoutingModule,
    HttpClientModule,
    GlobalModule,
    PagesModule,
    ModelsModule,
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
